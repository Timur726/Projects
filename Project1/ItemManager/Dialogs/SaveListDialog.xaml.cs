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
    public sealed partial class SaveListDialog : ContentDialog
    {
        private readonly ObservableCollection<Item> items;

        public SaveListDialog(ObservableCollection<Item> items)
        {
            IsPrimaryButtonEnabled = false;
            InitializeComponent();
            DataContext = new ListFile();
            this.items = items;
        }

        private void ContentDialog_PrimaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
            var serializedList = JsonConvert.SerializeObject(items, new JsonSerializerSettings{TypeNameHandling = TypeNameHandling.All});
            File.WriteAllText($@"{Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)}\{(DataContext as ListFile).Filename}.json", serializedList);
        }

        private void ContentDialog_SecondaryButtonClick(ContentDialog sender, ContentDialogButtonClickEventArgs args)
        {
        }

        private void SaveChange(object sender, TextChangedEventArgs e)
        {
            //cannot save until file name is input
            if (SaveBox.Text != "")
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
