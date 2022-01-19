using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ItemManager.Models;
using System.ComponentModel;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;

namespace ItemManager.ViewModels
{
    class MainViewModel : INotifyPropertyChanged
    {
        public Item SelectedItem
        {
            get;
            set;
        }

        public ObservableCollection<Item> Items     //all visible items
        {
            get;
            set;
        }

        public ObservableCollection<Item> RemovedItems  //used in search for items not currently visible
        {
            get;
            set;
        }

        public ObservableCollection<Item> CompletedItems    //hidden items when only showing incompletes
        {
            get;
            set;
        }

        public MainViewModel()
        {
            Items = new ObservableCollection<Item>();
            RemovedItems = new ObservableCollection<Item>();
            CompletedItems = new ObservableCollection<Item>();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void NotifyPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public void RemoveItem()    //when deleting item
        {
            Items.Remove(SelectedItem);
        }

        private void Order(Item item, bool priorityToggled) //used by Search() and IncompleteToggle() to reintroduce items to list in correct sorting
        {
            int index = 0;
            if (!priorityToggled)
            {
                while (index < Items.Count && Items[index].Start.CompareTo(item.Start) == -1)
                {
                    index++;
                }

                while (index < Items.Count && Items[index].Start.CompareTo(item.Start) == 0)
                {
                    if (Items[index].Priority.Length < item.Priority.Length)
                    {
                        Items.Insert(index, item);
                        break;
                    }
                    index++;
                }
            }
            else
            {
                while (index < Items.Count && Items[index].Priority.Length > item.Priority.Length)
                {
                    index++;
                }

                while (index < Items.Count && Items[index].Priority.Length == item.Priority.Length)
                {
                    if (Items[index].Start.CompareTo(item.Start) == 1)
                    {
                        Items.Insert(index, item);
                        break;
                    }
                    index++;
                }
            }

            if (index < Items.Count && !Items.Contains(item))
            {
                Items.Insert(index, item);
            }

            if (!Items.Contains(item))
            {
                Items.Add(item);
            }
        }

        public void Search(string query, bool priorityToggled)  //when searching
        {
            List<Item> AllItems = new List<Item>(Items);

            foreach (Item item in RemovedItems)
            {
                AllItems.Add(item);
            }

            foreach (Item item in AllItems) //hides or unhides item depending on whether it contains the search query
            {
                if (!item.Name.Contains(query, StringComparison.InvariantCultureIgnoreCase) &&
                    !item.Description.Contains(query, StringComparison.InvariantCultureIgnoreCase) &&
                    (item.ItemType == 0 || (item.ItemType == 1 && !item.Attendees.Contains(query, StringComparison.InvariantCultureIgnoreCase))))
                {
                    Items.Remove(item);
                    if (!RemovedItems.Contains(item))
                    {
                        RemovedItems.Add(item);
                    }
                }
                else
                {
                    if (!Items.Contains(item))
                    {
                        Order(item, priorityToggled);
                    }
                    RemovedItems.Remove(item);
                }
            }
        }

        public void IncompleteToggle(bool incompleteToggled, bool priorityToggled)  //when toggling showing only incomplete items
        {
            List<int> indices = new List<int>();

            if (!incompleteToggled)
            {
                foreach (Item item in Items)
                {
                    if (item.IsCompleted)
                    {
                        CompletedItems.Add(item);
                        indices.Add(Items.IndexOf(item));
                    }
                }
                for (int i = Items.Count - 1; i >= 0; i--)
                {
                    if (indices.Contains(i))
                    {
                        Items.RemoveAt(i);
                    }
                }
            }
            else
            {
                foreach (Item item in CompletedItems)
                {
                    Order(item, priorityToggled);
                }
                CompletedItems.Clear();
            }
        }

        public void PriorityToggle(bool priorityToggled)    //when toggling sorting by priority or date/time
        {
            List<Item> PrioritySorting = new List<Item>(Items);
            Items.Clear();

            if (!priorityToggled)
            {
                RemovedItems = new ObservableCollection<Item>(RemovedItems.OrderByDescending(i => i.Priority.Length));
                CompletedItems = new ObservableCollection<Item>(CompletedItems.OrderByDescending(i => i.Priority.Length));
                for (int i = 5; i > 0; i -= 2)
                {
                    foreach (Item item in PrioritySorting)
                    {
                        if(item.Priority.Length == i)
                        {
                            Items.Add(item);
                        }
                    }
                }
            }
            else
            {
                RemovedItems = new ObservableCollection<Item>(RemovedItems.OrderBy(i => i.Start));
                CompletedItems = new ObservableCollection<Item>(CompletedItems.OrderBy(i => i.Start));
                PrioritySorting = new List<Item>(PrioritySorting.OrderBy(i => i.Start));
                foreach (Item item in PrioritySorting)
                {
                    Items.Add(item);
                }
            }
        }
    }
}
