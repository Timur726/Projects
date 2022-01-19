using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml;

namespace ItemManager.Models
{
    public class ListFile : DependencyObject
    {
        public static readonly DependencyProperty FilenameProperty =
            DependencyProperty.Register("Filename", typeof(string), typeof(ListFile), new PropertyMetadata(null));

        public ListFile()
        {
            Filename = string.Empty;
        }

        public string Filename
        {
            get
            {
                return (string)GetValue(FilenameProperty);
            }
            set
            {
                SetValue(FilenameProperty, value);
            }
        }
    }
}
