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
using ItemManager.Dialogs;
using ItemManager.ViewModels;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace ItemManager
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private bool incompleteToggled; //if only showing incomplete items
        private bool priorityToggled;   //if sorting by priority rather than date/time

        public MainPage()
        {
            InitializeComponent();
            DataContext = new MainViewModel();
            incompleteToggled = false;
            priorityToggled = false;
        }

        private async void AddTaskClick(object sender, RoutedEventArgs e)   //when adding a new task
        {
            var diag = new AddTaskDialog((DataContext as MainViewModel).Items, priorityToggled);
            await diag.ShowAsync();
        }

        private async void AddAppClick(object sender, RoutedEventArgs e)    //when adding a new appointment
        {
            var diag = new AddAppDialog((DataContext as MainViewModel).Items, priorityToggled);
            await diag.ShowAsync();
        }

        private async void EditClick(object sender, RoutedEventArgs e)      //when editing an existing item
        {
            var mainViewModel = DataContext as MainViewModel;
            if (mainViewModel.SelectedItem != null && mainViewModel.SelectedItem.ItemType == 0)         //task
            {
                var diag = new AddTaskDialog(mainViewModel.Items, mainViewModel.SelectedItem, priorityToggled);
                await diag.ShowAsync();
            }
            else if (mainViewModel.SelectedItem != null && mainViewModel.SelectedItem.ItemType == 1)    //appointment
            {
                var diag = new AddAppDialog(mainViewModel.Items, mainViewModel.SelectedItem, priorityToggled);
                await diag.ShowAsync();
            }
        }

        private void DeleteClick(object sender, RoutedEventArgs e)          //when deleting a selected item
        {
            (DataContext as MainViewModel).RemoveItem();
        }

        private async void LoadClick(object sender, RoutedEventArgs e)      //when loading an existing item list
        {
            var diag = new LoadListDialog((DataContext as MainViewModel).Items);
            await diag.ShowAsync();
            (DataContext as MainViewModel).CompletedItems.Clear();
            (DataContext as MainViewModel).RemovedItems.Clear();
            //resetting searchbar and toggles to default settings
            SearchBar.Text = "";
            IncompleteButton.Content = "Showing All Items";
            incompleteToggled = false;
            PriorityButton.Content = "Sorting By Date/Time";
            priorityToggled = false;
            AddTaskButton.Opacity = 1.0;
            AddAppButton.Opacity = 1.0;
            AddTaskButton.IsHitTestVisible = true;
            AddAppButton.IsHitTestVisible = true;
        }

        private async void SaveClick(object sender, RoutedEventArgs e)      //when saving an item list
        {
            //resetting searchbar and toggles to default settings before save
            (DataContext as MainViewModel).Search("", priorityToggled);

            if (incompleteToggled)
            {
                (DataContext as MainViewModel).IncompleteToggle(incompleteToggled, priorityToggled);
            }
            if (priorityToggled)
            {
                (DataContext as MainViewModel).PriorityToggle(priorityToggled);
            }
            var diag = new SaveListDialog((DataContext as MainViewModel).Items);
            await diag.ShowAsync();

            //resetting searchbar and toggles to previous settings after save
            if (incompleteToggled)
            {
                (DataContext as MainViewModel).IncompleteToggle(!incompleteToggled, priorityToggled);
            }
            if (priorityToggled)
            {
                (DataContext as MainViewModel).PriorityToggle(!priorityToggled);
            }

            (DataContext as MainViewModel).Search(SearchBar.Text, priorityToggled);
        }

        private void ListSearched(object sender, TextChangedEventArgs e)    //when searching the item list
        {
            (DataContext as MainViewModel).Search(SearchBar.Text, priorityToggled);

            if(SearchBar.Text != "")    //turning off add buttons when searching
            {
                AddTaskButton.Opacity = 0.5;
                AddAppButton.Opacity = 0.5;
            }
            else                        //turning on add buttons when not searching
            {
                AddTaskButton.Opacity = 1.0;
                AddAppButton.Opacity = 1.0;
            }
            AddTaskButton.IsHitTestVisible = SearchBar.Text.Equals("");
            AddAppButton.IsHitTestVisible = SearchBar.Text.Equals("");
        }

        private void IncompleteToggleClick(object sender, RoutedEventArgs e)    //when showing only incomplete items
        {
            (DataContext as MainViewModel).IncompleteToggle(incompleteToggled, priorityToggled);

            //toggling button text
            if(!incompleteToggled)
            {
                IncompleteButton.Content = "Showing Incomplete Items";
            }
            else
            {
                IncompleteButton.Content = "Showing All Items";
            }
            incompleteToggled ^= true;
        }

        private void PriorityToggleClick(object sender, RoutedEventArgs e)      //when toggling sort (priority or date/time)
        {
            (DataContext as MainViewModel).PriorityToggle(priorityToggled);

            //toggling button text
            if(!priorityToggled)
            {
                PriorityButton.Content = "Sorting By Priority";
            }
            else
            {
                PriorityButton.Content = "Sorting By Date/Time";
            }
            priorityToggled ^= true;
        }
    }
}
