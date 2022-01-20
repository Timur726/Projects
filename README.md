# Projects

PROJECT 1 - Item Manager

The "Item Manager" is a Windows app in which a user can keep track of their tasks and appointments. The app was designed in Microsoft Visual Studio 2019 using C# and XAML.

The app was designed around the MVVM architectural pattern. The main model is the Item class, which can exist as either a Task object or an Appointment object. There is also the ListFile model, which is used for saving data. The views are the five .xaml files: four dialogs and the main page. Each .xaml file has a respective .xaml.cs file for the code-behind. The view model, MainViewModel.cs, houses functions used by MainPage.xaml.cs to interact with the data.

To run the app, download all of the provided files and open ItemManager.sln in Visual Studio. Change the solution platform at the top from ARM to x64, and press the green play button next to "Local Machine".

There are 8 buttons and a search bar.

"Add Task" - Add a task with name, description, deadline, and priority level.

"Add Appointment" - Add an appointment with name, description, start date, end date, list of attendees, and priority level.

"Edit Item" - Edit data for an item. Must select item first.

"Delete Item" - Delete item. Must select item first.

"Load List" - Load list of items that was saved previously.

"Save List" - Save current list of items. The list will be saved locally via JSON serialization.

"Showing All Items / Showing Incomplete Items" - Toggle to show all items or items that have not been checked off as complete.

"Sorting By Date/Time / Sorting By Priority" - Toggle to sort items by date/time or by priority.

You can check off items as "Complete" on the left of each item.
_____

PROJECT 2 - Elevator Scheduler

The "Elevator Scheduler" is a kernel module that simulates an elevator. It is controlled by calls from the operating system (which must be manually inserted for the program to work) which add random amounts of people to different floors. It is written entirely in C and should be run on a Linux machine.

The elevator seeks to drop everyone off to the correct floor in the shortest possible time. There are 10 floors, each containing a linked list of people. The people on the elevator are also stored as a linked list. There are three types of people: one weighing 150 pounds, another weighing 170 pounds, and another weighing 225 pounds. These people are added to the floors through a system call. Each person has a starting floor, a destination floor, and a weight.

The constraints are as follows. The elevator can hold a maximum of 1000 pounds. Each up or down movement takes one second. Stopping on a floor to load and unload takes two seconds. The people on each floor are served first in, first out.

The elevator uses SCAN scheduling. This means that the elevator moves from floor 1 to 10, servicing requests along the way. Then, it goes from floor 10 to 1, servicing requests along the way. If it is going up, it will only pick up people who are going up, and vice versa. It will stop picking people up on that floor when (1) the elevator cannot hold the next person's weight, (2) the next person is going in the other direction, or (3) the floor is empty. The elevator will turn around if there is no one left to drop off and all the floors in its current direction are empty.
