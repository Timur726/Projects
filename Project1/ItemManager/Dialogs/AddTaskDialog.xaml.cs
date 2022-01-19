using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using ItemManager.Models;

// The Content Dialog item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace ItemManager.Dialogs
{
    public sealed partial class AddTaskDialog : ContentDialog
    {
        private readonly IList<Item> items;

        private DateTime deadline;
        private readonly bool priorityToggled;

        public AddTaskDialog(IList<Item> items, bool priorityToggled)   //adding task
        {
            IsPrimaryButtonEnabled = false;
            InitializeComponent();
            DataContext = new Task();
            this.items = items;
            this.priorityToggled = priorityToggled;
            //setting up default data
            deadline = new DateTime(2020, 1, 1, 0, 0, 0);
            DeadlineDate.SelectedDate = deadline.Date;
            DeadlineTime.Time = deadline.TimeOfDay;
        }

        public AddTaskDialog(IList<Item> items, Item taskToEdit, bool priorityToggled)  //editing task
        {
            InitializeComponent();
            DataContext = taskToEdit;
            this.items = items;
            this.priorityToggled = priorityToggled;
            Title = "Edit Task";
            PrimaryButtonText = "Edit";
            //setting up data from previous task
            deadline = taskToEdit.Start;
            NameBox.Text = taskToEdit.Name;
            DescriptionBox.Text = taskToEdit.Description;
            DeadlineDate.SelectedDate = taskToEdit.Start.Date;
            DeadlineTime.Time = taskToEdit.Start.TimeOfDay;
            PriorityBox.SelectedItem = taskToEdit.Priority;
        }

        private void ContentDialog_PrimaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
            var taskToEdit = DataContext as Task;
            var existingTask = items.FirstOrDefault(t => t.ItemID == taskToEdit.ItemID);
            int index = 0;

            //same as Order() function in MainViewModel.cs

            if(existingTask != null)
            {
                items.RemoveAt(items.IndexOf(existingTask));
            }

            taskToEdit.Start = deadline;

            if (!priorityToggled)
            {
                while (index < items.Count && items[index].Start.CompareTo(taskToEdit.Start) == -1)
                {
                    index++;
                }

                while (index < items.Count && items[index].Start.CompareTo(taskToEdit.Start) == 0)
                {
                    if (items[index].Priority.Length < taskToEdit.Priority.Length)
                    {
                        items.Insert(index, taskToEdit);
                        break;
                    }
                    index++;
                }
            }
            else
            {
                while (index < items.Count && items[index].Priority.Length > taskToEdit.Priority.Length)
                {
                    index++;
                }

                while (index < items.Count && items[index].Priority.Length == taskToEdit.Priority.Length)
                {
                    if (items[index].Start.CompareTo(taskToEdit.Start) == 1)
                    {
                        items.Insert(index, taskToEdit);
                        break;
                    }
                    index++;
                }
            }

            if (index < items.Count && !items.Contains(taskToEdit))
            {
                items.Insert(index, taskToEdit);
            }

            if (!items.Contains(taskToEdit))
            {
                items.Add(taskToEdit);
            }
        }

        private void ContentDialog_SecondaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
        }

        private void NameChange(object sender, TextChangedEventArgs e)
        {
            if (NameBox.Text != "" && DescriptionBox.Text != "")
            {
                IsPrimaryButtonEnabled = true;
            }
            else
            {
                IsPrimaryButtonEnabled = false;
            }
        }

        private void DescriptionChange(object sender, TextChangedEventArgs e)
        {
            if (NameBox.Text != "" && DescriptionBox.Text != "")
            {
                IsPrimaryButtonEnabled = true;
            }
            else
            {
                IsPrimaryButtonEnabled = false;
            }
        }

        private void DeadlineTimeChange(TimePicker t, TimePickerSelectedValueChangedEventArgs e)
        {
            deadline = new DateTime(deadline.Year, deadline.Month, deadline.Day, e.NewTime.Value.Hours, e.NewTime.Value.Minutes, e.NewTime.Value.Seconds);
        }

        private void DeadlineDateChange(DatePicker d, DatePickerSelectedValueChangedEventArgs e)
        {
            deadline = new DateTime(e.NewDate.Value.Year, e.NewDate.Value.Month, e.NewDate.Value.Day, deadline.Hour, deadline.Minute, deadline.Second);
        }
    }
}
