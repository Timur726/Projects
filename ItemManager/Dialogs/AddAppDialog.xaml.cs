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
    public sealed partial class AddAppDialog : ContentDialog
    {
        private readonly IList<Item> items;

        private DateTime start;
        private DateTime stop;
        private readonly bool priorityToggled;

        public AddAppDialog(IList<Item> items, bool priorityToggled)    //adding task
        {
            IsPrimaryButtonEnabled = false;
            InitializeComponent();
            DataContext = new Appointment();
            this.items = items;
            this.priorityToggled = priorityToggled;
            //setting up default data
            start = new DateTime(2020, 1, 1, 0, 0, 0);
            stop = new DateTime(2020, 1, 1, 0, 0, 0);
            StartDate.SelectedDate = start.Date;
            StartTime.Time = start.TimeOfDay;
            StopDate.SelectedDate = stop.Date;
            StopTime.Time = stop.TimeOfDay;
        }

        public AddAppDialog(IList<Item> items, Item appToEdit, bool priorityToggled)    //editing task
        {
            InitializeComponent();
            DataContext = appToEdit;
            this.items = items;
            this.priorityToggled = priorityToggled;
            Title = "Edit Appointment";
            PrimaryButtonText = "Edit";
            //setting up data from previous appointment
            start = appToEdit.Start;
            stop = appToEdit.Stop;
            NameBox.Text = appToEdit.Name;
            DescriptionBox.Text = appToEdit.Description;
            StartDate.SelectedDate = appToEdit.Start.Date;
            StartTime.Time = appToEdit.Start.TimeOfDay;
            StopDate.SelectedDate = appToEdit.Stop.Date;
            StopTime.Time = appToEdit.Stop.TimeOfDay;
            PriorityBox.SelectedItem = appToEdit.Priority;
        }

        private void ContentDialog_PrimaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
            var appToEdit = DataContext as Appointment;
            var existingApp = items.FirstOrDefault(t => t.ItemID == appToEdit.ItemID);
            int index = 0;

            //same as Order() function in MainViewModel.cs

            if (existingApp != null)
            {
                items.RemoveAt(items.IndexOf(existingApp));
            }

            appToEdit.Start = start;
            appToEdit.Stop = stop;

            if (!priorityToggled)
            {
                while (index < items.Count && items[index].Start.CompareTo(appToEdit.Start) == -1)
                {
                    index++;
                }

                while (index < items.Count && items[index].Start.CompareTo(appToEdit.Start) == 0)
                {
                    if (items[index].Priority.Length < appToEdit.Priority.Length)
                    {
                        items.Insert(index, appToEdit);
                        break;
                    }
                    index++;
                }
            }
            else
            {
                while (index < items.Count && items[index].Priority.Length > appToEdit.Priority.Length)
                {
                    index++;
                }

                while (index < items.Count && items[index].Priority.Length == appToEdit.Priority.Length)
                {
                    if (items[index].Start.CompareTo(appToEdit.Start) == 1)
                    {
                        items.Insert(index, appToEdit);
                        break;
                    }
                    index++;
                }
            }

            if (index < items.Count && !items.Contains(appToEdit))
            {
                items.Insert(index, appToEdit);
            }

            if (!items.Contains(appToEdit))
            {
                items.Add(appToEdit);
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

        private void StartTimeChange(TimePicker t, TimePickerSelectedValueChangedEventArgs a)
        {
            start = new DateTime(start.Year, start.Month, start.Day, a.NewTime.Value.Hours, a.NewTime.Value.Minutes, a.NewTime.Value.Seconds);
        }

        private void StartDateChange(DatePicker d, DatePickerSelectedValueChangedEventArgs a)
        {
            start = new DateTime(a.NewDate.Value.Year, a.NewDate.Value.Month, a.NewDate.Value.Day, start.Hour, start.Minute, start.Second);
        }

        private void StopTimeChange(TimePicker t, TimePickerSelectedValueChangedEventArgs a)
        {
            stop = new DateTime(stop.Year, stop.Month, stop.Day, a.NewTime.Value.Hours, a.NewTime.Value.Minutes, a.NewTime.Value.Seconds);
        }

        private void StopDateChange(DatePicker d, DatePickerSelectedValueChangedEventArgs a)
        {
            stop = new DateTime(a.NewDate.Value.Year, a.NewDate.Value.Month, a.NewDate.Value.Day, stop.Hour, stop.Minute, stop.Second);
        }
    }
}