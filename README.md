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
