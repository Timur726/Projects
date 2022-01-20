#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/linkage.h>
#include <stdbool.h>

MODULE_LICENSE("GPL");

#define mallocFlags (__GFP_RECLAIM | __GFP_IO | __GFP_FS)

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL

#define DAILY_WORKER 0
#define MAINTENANCE_PERSON 1
#define MAIL_CARRIER 2

#define DAILY_WORKER_WEIGHT 150
#define MAINTENANCE_PERSON_WEIGHT 170
#define MAIL_CARRIER_WEIGHT 225

#define PEOPLE_TYPES 3
#define MAX_WEIGHT 1000

struct list_head floors[10];	//array of lists containing people on floor
static int floor_population[10];	//number of people per floors
int num_waiting;	//number of people awaiting elevator
int num_serviced;	//number of people who have left elevator

bool has_run_once;	//for logic; true after first person is loaded
bool is_deactivating1, is_deactivating2;	//for stopping; unload but not load
bool has_stopped_before;        //to create kthread outside init if start runs again

static char *message;	//for printing
static int read_p;	//for printing

struct person {
        int type;	//as defined above (0, 1, or 2)
        int weight;	//as defined above (150, 170, or 225)
        int start;      //person's starting floor (between 0 and 9; prints as 1-10)
        int dest;       //person's destination floor (between 0 and 9; prints as 1-10)
        char name;	//'D', 'M', and 'C'
        struct list_head list;	//to reference person
};

struct {
        int current_weight;	//up to 1000
        int current_floor;      //between 0 and 9 (print as 1-10)
        int num_passengers;
        char state[8];		//"OFFLINE", "IDLE", "LOADING", "UP", "DOWN"
        char general_direction[5];  //to remember direction when loading; "UP" (default), "DOWN"
        int num_type[PEOPLE_TYPES];        //number of each type of person on the elevator
        struct list_head list;	//list of people on elevator
} elevator;

static struct file_operations fops;

struct task_struct *kthread;

struct mutex elevator_mutex;

/**********/

int add_to_floor(int start, int dest, int type)
{
	//called within issue_request; adds person to floor
	//return 1 if invalid type, 0 if success
        int weight;
        char name;
        struct person *p;

        switch(type)
        {
                case DAILY_WORKER:
                        weight = DAILY_WORKER_WEIGHT;
                        name = 'D';
                        break;
                case MAINTENANCE_PERSON:
                        weight = MAINTENANCE_PERSON_WEIGHT;
                        name = 'M';
                        break;
                case MAIL_CARRIER:
                        weight = MAIL_CARRIER_WEIGHT;
                        name = 'C';
                        break;
                default:
                        return 1;
        }

        p = kmalloc(sizeof(struct person), mallocFlags);
        if(p == NULL)
                return -ENOMEM;

        p->type = type;
        p->weight = weight;
        p->name = name;
        p->start = start-1;     //stores start between 0 and 9
        p->dest = dest-1;       //stores destination between 0 and 9

        mutex_lock_interruptible(&elevator_mutex);
        list_add_tail(&p->list, &(floors[start-1]));	//adds to floor
        num_waiting++;
        floor_population[start-1]++;
        mutex_unlock(&elevator_mutex);
        return 0;
}

int add_to_elevator(struct person *p)
{
	//called in manage_elevator() when loading
	//return -1 if too heavy, 0 if success
        struct person *temp;
        if(p->type == DAILY_WORKER)
        {
                if(elevator.current_weight > MAX_WEIGHT - DAILY_WORKER_WEIGHT)
                        return -1;
                else
                        elevator.current_weight += DAILY_WORKER_WEIGHT;
        }
        if(p->type == MAINTENANCE_PERSON)
        {
                if(elevator.current_weight > MAX_WEIGHT - MAINTENANCE_PERSON_WEIGHT)
                        return -1;
                else
                        elevator.current_weight += MAINTENANCE_PERSON_WEIGHT;
        }
        if(p->type == MAIL_CARRIER)
        {
                if(elevator.current_weight > MAX_WEIGHT - MAIL_CARRIER_WEIGHT)
                        return -1;
                else
                        elevator.current_weight += MAIL_CARRIER_WEIGHT;
        }

	//temp to push into elevator
        temp = kmalloc(sizeof(struct person), mallocFlags);
        temp->type = p->type;
        temp->weight = p->weight;
        temp->start = p->start;
        temp->dest = p->dest;
        temp->name = p->name;

        elevator.num_type[p->type]++;
        list_add_tail(&temp->list, &elevator.list);
        elevator.num_passengers++;

        //deletes person from floor
        list_del(&p->list);
        num_waiting--;
        floor_population[elevator.current_floor]--;

        return 0;
}

int remove_from_elevator(void)
{
	//called in manage_elevator to unload
	//returns number of people removed (0 or more)
        struct list_head *temp;
        struct person *p;
        int people_removed = 0;
        int r;
        bool loop = true;
        while(loop) //while loop necessary, or else removing from list will break list_for_each
        {
                r = 0;
                mutex_lock_interruptible(&elevator_mutex);
                list_for_each(temp, &elevator.list)
                {
                        p = list_entry(temp, struct person, list);
                        if(p->dest == elevator.current_floor)
                        {
                                elevator.num_type[p->type]--;
                                if(p->type == DAILY_WORKER)
                                        elevator.current_weight -= DAILY_WORKER_WEIGHT;
                                if(p->type == MAINTENANCE_PERSON)
                                        elevator.current_weight -= MAINTENANCE_PERSON_WEIGHT;
                                if(p->type == MAIL_CARRIER)
                                        elevator.current_weight -= MAIL_CARRIER_WEIGHT;
                                list_del(&p->list);
                                kfree(p);
                                elevator.num_passengers--;
                                num_serviced++;
                                people_removed++;
                                r++;
                                break;
                        }
                }
                mutex_unlock(&elevator_mutex);
                if(r == 0)
                {
                        loop = false;
                        break;
                }
        }

        return people_removed;
}

int manage_elevator(void *ptr)
{
	//manages all movements and functions of elevator (except those handled above)
	//called by kthread, runs until kthread stops
        struct list_head *temp;
        struct person *p;
        int s = 0;	//for checking weight
        int i;		//counter for for loops
        bool loop = true;	//flag for while loops
	bool turn_around = false;	//to turn around if there is no need to continue in current direction

        while(!kthread_should_stop())
        {
                if(is_deactivating1 && elevator.num_passengers == 0)
                {
			//if consumer.c called with --stop, and elevator has unloaded
			mutex_lock_interruptible(&elevator_mutex);
                        is_deactivating1 = false;
			mutex_unlock(&elevator_mutex);
                        return 0;
                }

		mutex_lock_interruptible(&elevator_mutex);
                if(has_run_once && num_waiting == 0 && elevator.num_passengers == 0 &&
                   (elevator.current_floor == 4 || elevator.current_floor == 5))
                        strcpy(elevator.state, "IDLE");	//after unloading and calling consumer.c with --stop
                else if(has_run_once && num_waiting == 0 && elevator.num_passengers == 0 && elevator.current_floor < 4)
                {
			//stop elevator on floor 5 if stopped on bottom half; probably optimal
			strcpy(elevator.state, "LOADING");
			ssleep(2);	//otherwise it will not wait to unload the last person(s)
                        strcpy(elevator.state, "UP");
                        while(elevator.current_floor != 4 && num_waiting == 0)
                        {
				//ascend to floor 5
                                ssleep(1);
                                elevator.current_floor++;
                        }
                        strcpy(elevator.state, "IDLE");
                }
                else if(has_run_once && num_waiting == 0 && elevator.num_passengers == 0 && elevator.current_floor > 5)
                {
			//stop elevator on floor 6 if stopped on top half; probably optimal
			strcpy(elevator.state, "LOADING");
			ssleep(2);	//otherwise it will not wait to unload the last person(s)
                        strcpy(elevator.state, "DOWN");
                        while(elevator.current_floor != 5 && num_waiting == 0)
                        {
				//descend to floor 6
                                ssleep(1);
                                elevator.current_floor--;
                        }
                        strcpy(elevator.state, "IDLE");
                }
		mutex_unlock(&elevator_mutex);

                if(strcmp(elevator.state, "IDLE") == 0 && num_waiting != 0)
                {
                        if(has_run_once)
                        {
				//set general direction or state
                                if(elevator.current_floor == 0)
                                        strcpy(elevator.general_direction, "UP");
                                if(elevator.current_floor == 9)
                                        strcpy(elevator.general_direction, "DOWN");
                                if(floor_population[elevator.current_floor] != 0)
                                        strcpy(elevator.state, elevator.general_direction);
                        }
                        else
                                ssleep(1);      //otherwise it will miss the first floor due to order of issue requests
                        for(i = 0; i < 10; i++)
                        {
				//go through each floor, and see if there are people waiting
                                if(strcmp(elevator.state, "IDLE") != 0)
                                        break;
                                if(floor_population[i] != 0 && i == elevator.current_floor)
                                {
                                        strcpy(elevator.state, "LOADING");
                                        has_run_once = true;
                                        break;
                                }
                                else if(floor_population[i] != 0 && i < elevator.current_floor)
                                {
                                        strcpy(elevator.state, "DOWN");
                                        has_run_once = true;
                                        break;
                                }
                                else if(floor_population[i] != 0 && i > elevator.current_floor)
                                {
                                        strcpy(elevator.state, "UP");
                                        has_run_once = true;
                                        break;
                                }
                        }
                }
                else if(strcmp(elevator.state, "LOADING") == 0)
                {
			//only handles loading; unloading is handled further below (but earlier)
                        loop = true;
                        if(elevator.current_floor == 0)
                                strcpy(elevator.general_direction, "UP");
                        if(elevator.current_floor == 9)
                                strcpy(elevator.general_direction, "DOWN");
                        while(loop && !is_deactivating2)
                        {
				//check if there are people to load
                                list_for_each(temp, &floors[elevator.current_floor])
                                {
                                        p = list_entry(temp, struct person, list);

                                        if((strcmp(elevator.general_direction,"UP") == 0 &&
                                           p->dest > elevator.current_floor) ||
                                           (strcmp(elevator.general_direction,"DOWN") == 0
                                           && p->dest < elevator.current_floor))
                                        {
                                                s = add_to_elevator(p);	//attempt to load next person
                                                if(s != 0)
                                                        loop = false;   //next person too heavy
                                                break;
                                        }
                                        else if(p->start == p->dest)
                                        {
						//if person is leaving on same floor that they started on
                                                list_del(&p->list);
                                                num_waiting--;
                                                floor_population[elevator.current_floor]--;
                                                num_serviced++;
                                                kfree(p);
                                                break;
                                        }
                                        else
                                        {
						//next person moving in other direction
                                                loop = false;
                                                break;
                                        }
                                }
                                if(floor_population[elevator.current_floor] == 0)
                                        break;	//empty floor, so stop looping
                        }
                        ssleep(2);	//2 seconds to load
                        strcpy(elevator.state, elevator.general_direction);	//continue moving
                }
                else if(strcmp(elevator.state, "UP") == 0 || strcmp(elevator.state, "DOWN") == 0)
                {
                        ssleep(1);	//1 second to move
			if(strcmp(elevator.state, "UP") == 0)
                        	elevator.current_floor++;
			if(strcmp(elevator.state, "DOWN") == 0)
				elevator.current_floor--;

                        if(remove_from_elevator() != 0)
				strcpy(elevator.state, "LOADING");	//if unloaded someone, change state
			turn_around = true;	//assumes elevator will change direction
			list_for_each(temp, &floors[elevator.current_floor])
			{
				p = list_entry(temp, struct person, list);
				if(floor_population[elevator.current_floor] != 0 && p->weight <= MAX_WEIGHT - elevator.current_weight)
				{
					if((p->dest >= elevator.current_floor && strcmp(elevator.general_direction, "UP") == 0) ||
					   (p->dest <= elevator.current_floor && strcmp(elevator.general_direction, "DOWN") == 0) ||
					   elevator.current_floor == 0 || elevator.current_floor == 9)
					{
						strcpy(elevator.state, "LOADING");
						turn_around = false;	//if there are still people to load, do not change direction
					}
				}
				break;
			}

			if(turn_around && elevator.num_passengers == 0)
			{
				//if no one left to load on this floor, and elevator is empty
				if(strcmp(elevator.general_direction, "UP") == 0)
				{
					for(i = elevator.current_floor + 1; i < 10; i++)
					{
						if(floor_population[i] != 0)
							turn_around = false;	//if more people to pick up later, don't change direction
					}
					if(turn_around) //if elevator is turning around
					{
						strcpy(elevator.general_direction, "DOWN");
						list_for_each(temp, &floors[elevator.current_floor])
			                        {
                        			        p = list_entry(temp, struct person, list);

							if(p->dest <= elevator.current_floor && strcmp(elevator.general_direction, "DOWN") == 0)
								strcpy(elevator.state, "LOADING");	//people on this floor going other way
							break;
						}
						if(strcmp(elevator.state, "LOADING") != 0)
						{
							ssleep(2);	//2 seconds to load (needed for logic to work)
							strcpy(elevator.state, "DOWN");
						}
					}
				}
				else if(strcmp(elevator.general_direction, "DOWN") == 0)
				{
					for(i = elevator.current_floor - 1; i >= 0; i--)
					{
						if(floor_population[i] != 0)
							turn_around = false;	//if more people to pick up later, don't change direction
					}
					if(turn_around)	//if elevator is turning around
					{
						strcpy(elevator.general_direction, "UP");
						list_for_each(temp, &floors[elevator.current_floor])
						{
							p = list_entry(temp, struct person, list);

							if(p->dest >= elevator.current_floor && strcmp(elevator.general_direction, "UP") == 0)
								strcpy(elevator.state, "LOADING");	//people on this floor going other way
							break;
						}
						if(strcmp(elevator.state, "LOADING") != 0)
						{
							ssleep(2);	//2 seconds to load (needed for logic to work)
							strcpy(elevator.state, "UP");
						}
					}
				}
			}

			//always turn around at floors 1 and 10
			if(elevator.current_floor == 0 && strcmp(elevator.state, "LOADING") != 0)
			{
				strcpy(elevator.general_direction, "UP");
				strcpy(elevator.state, "UP");
			}
                        if(elevator.current_floor == 9 && strcmp(elevator.state, "LOADING") != 0)
                        {
                                strcpy(elevator.general_direction, "DOWN");
                                strcpy(elevator.state, "DOWN");
                        }
                }
        }

        return 0;
}

/**********/

int print_queue(void)
{
	//print status of elevator system
        struct person *p;
        struct list_head *temp;
        int i = 9;
        char *buf = kmalloc(sizeof(char) * 1000, __GFP_RECLAIM);

        if(buf == NULL)
                return -ENOMEM;

        strcpy(message, "");

        sprintf(buf, "Elevator state: %s\n", elevator.state);
        strcat(message, buf);
        sprintf(buf, "Current floor: %d\n", elevator.current_floor + 1);
        strcat(message, buf);
        sprintf(buf, "Current weight: %d\n", elevator.current_weight);
        strcat(message, buf);
        sprintf(buf, "Elevator status: %d D, %d M, %d C\n",
                elevator.num_type[0], elevator.num_type[1], elevator.num_type[2]);
        strcat(message, buf);
        sprintf(buf, "Number of passengers: %d\n", elevator.num_passengers);
        strcat(message, buf);
        sprintf(buf, "Number of passengers waiting: %d\n", num_waiting);
        strcat(message, buf);
        sprintf(buf, "Number passengers serviced: %d\n\n\n", num_serviced);
        strcat(message, buf);

        for(i = 9; i >= 0; i--)
        {
		//iterate through each floor and print its info
                sprintf(buf, "[");
                strcat(message, buf);
                if(i == elevator.current_floor)
                {
                        sprintf(buf, "*");
                        strcat(message, buf);
                }
                else
                {
                        sprintf(buf, " ");
                        strcat(message, buf);
                }
                sprintf(buf, "] Floor %d: %d ", i+1, floor_population[i]);
                strcat(message, buf);
                list_for_each(temp, &floors[i])
                {
			//print each person on floor
                        p = list_entry(temp, struct person, list);
                        sprintf(buf, "%c ", p->name);
                        strcat(message, buf);
                }
                sprintf(buf, "\n");
                strcat(message, buf);
        }

        strcat(message, "\n");

        kfree(buf);

        return 0;
}

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file)
{
	//upon calling cat /proc/elevator
        read_p = 1;
        message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
        if(message == NULL)
                return -ENOMEM;

        return print_queue();
}

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{
        int len = strlen(message);
        read_p = !read_p;
        if(read_p)
                return 0;
        copy_to_user(buf, message, len);
        return len;
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file)
{
        kfree(message);
        return 0;
}

/**********/

extern long (*STUB_start_elevator)(void);
long start_elevator(void)
{
	//upon calling consumer.c with --start
        if(strcmp(elevator.state, "OFFLINE") != 0)
                return 1;
        else
        {
                strcpy(elevator.state, "IDLE");
                if(has_stopped_before)
                        kthread = kthread_run(manage_elevator, NULL, "manage elevator");
                return 0;
        }
}

extern long (*STUB_issue_request)(int,int,int);
long issue_request(int start_floor, int destination_floor, int type)
{
	//upon calling producer.c with integer
        return add_to_floor(start_floor, destination_floor, type);
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void)
{
	//upon calling consumer.c with --stop
        if(strcmp(elevator.state, "OFFLINE") == 0 || is_deactivating2)
                return 1;
        is_deactivating1 = true;        //tells manage_elevator loop when to break
        is_deactivating2 = true;        //makes sure no one can be loaded while deactivating
        while(is_deactivating1)
        {
                //waits for all passengers to get off
		printk(" ");	//necessary, or while loop doesn't work
        }
        kthread_stop(kthread);
        is_deactivating2 = false;	//finished deactivating
        strcpy(elevator.state, "OFFLINE");
        has_stopped_before = true;
        return 0;
}

int sys_init(void)
{
        STUB_start_elevator = &(start_elevator);
        STUB_issue_request = &(issue_request);
        STUB_stop_elevator = &(stop_elevator);
        return 0;
}

void sys_exit(void)
{
        STUB_start_elevator = NULL;
        STUB_issue_request = NULL;
        STUB_stop_elevator = NULL;
        return;
}

/**********/

static int elevator_init(void)
{
	//initialize all global variables, kthread, mutex, system call functions
	int i = 0;

	mutex_init(&elevator_mutex);
	mutex_lock_interruptible(&elevator_mutex);

        fops.read = elevator_proc_read;
        fops.open = elevator_proc_open;
        fops.release = elevator_proc_release;

        read_p = 0;

        num_waiting = 0;
        num_serviced = 0;

        strcpy(elevator.state, "OFFLINE");
        strcpy(elevator.general_direction, "UP");

        elevator.current_floor = 0;
        elevator.num_passengers = 0;
        elevator.current_weight = 0;
        INIT_LIST_HEAD(&elevator.list);
        elevator.num_type[0] = 0;
        elevator.num_type[1] = 0;
        elevator.num_type[2] = 0;

        has_run_once = false;
        is_deactivating1 = false;
        is_deactivating2 = false;
        has_stopped_before = false;

        for(i = 0; i < 10; i++)
                floor_population[i] = 0;

        for(i = 0; i < 10; i++)
                INIT_LIST_HEAD(&floors[i]);

        mutex_unlock(&elevator_mutex);

        kthread = kthread_run(manage_elevator, NULL, "manage elevator");

        sys_init();

        if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
        {
                remove_proc_entry(ENTRY_NAME, NULL);
                return -ENOMEM;
        }

        return 0;
}

module_init(elevator_init);

static void elevator_exit(void)
{
        remove_proc_entry(ENTRY_NAME, NULL);

        sys_exit();
        mutex_destroy(&elevator_mutex);

        return;
}

module_exit(elevator_exit);
