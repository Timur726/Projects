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
using Newtonsoft.Json;
using System.Collections.ObjectModel;

// The Content Dialog item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace ItemManager.Dialogs
{
    public sealed partial class LoadListDialog : ContentDialog
    {
        private readonly ObservableCollection<Item> items;

        public LoadListDialog(ObservableCollection<Item> items)
        {
            IsPrimaryButtonEnabled = false;
            InitializeComponent();
            DataContext = new ListFile();
            this.items = items;
        }

        private void ContentDialog_PrimaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
            if (File.Exists($@"{Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)}\{(DataContext as ListFile).Filename}.json"))
            {
                items.Clear();
                ObservableCollection<Item> deserializedList = JsonConvert.DeserializeObject<ObservableCollection<Item>>(File.ReadAllText($@"{Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)}\{(DataContext as ListFile).Filename}.json"));
                foreach(var item in deserializedList)
                {
                    DataContext = item;
                    if (item.ItemType == 0)
                    {
                        var task = new Task()
                        {
                            Name = item.Name,
                            Description = item.Description,
                            Start = item.Start,
                            ItemID = item.ItemID,
                            IsCompleted = item.IsCompleted,
                            Priority = item.Priority
                        };
                        items.Add(task);
                    }
                    else if (item.ItemType == 1)
                    {
                        var appointment = new Appointment()
                        {
                            Name = item.Name,
                            Description = item.Description,
                            Start = item.Start,
                            Stop = item.Stop,
                            Attendees = item.Attendees,
                            ItemID = item.ItemID,
                            IsCompleted = item.IsCompleted,
                            Priority = item.Priority
                        };
                        items.Add(appointment);
                    }
                }
            }
        }

        private void ContentDialog_SecondaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
        }

        private void LoadChange(object sender, TextChangedEventArgs e)
        {
            //cannot load until file name is input
            if (LoadBox.Text != "")
            {
                IsPrimaryButtonEnabled = true;
            }
            else
            {
                IsPrimaryButtonEnabled = false;
            }
        }
    }
}
