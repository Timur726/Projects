using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml;

namespace ItemManager.Models
{
    public class Item : DependencyObject
    {
        public static readonly DependencyProperty PriorityProperty =
            DependencyProperty.Register("Priority", typeof(string), typeof(Item), new PropertyMetadata(null));

        public static readonly DependencyProperty NameProperty =
            DependencyProperty.Register("Name", typeof(string), typeof(Item), new PropertyMetadata(null));

        public static readonly DependencyProperty DescriptionProperty =
            DependencyProperty.Register("Description", typeof(string), typeof(Item), new PropertyMetadata(null));

        public static readonly DependencyProperty IsCompletedProperty =
            DependencyProperty.Register("IsCompleted", typeof(bool), typeof(Item), new PropertyMetadata(null));

        public static readonly DependencyProperty StartProperty =
            DependencyProperty.Register("Start", typeof(string), typeof(Item), new PropertyMetadata(null));

        public static readonly DependencyProperty StopProperty =
            DependencyProperty.Register("Stop", typeof(string), typeof(Appointment), new PropertyMetadata(null));

        public static readonly DependencyProperty AttendeesProperty =
            DependencyProperty.Register("Attendees", typeof(string), typeof(Appointment), new PropertyMetadata(null));

        public Item()
        {
            ItemID = IDcounter;
            Priority = "!";
            Name = string.Empty;
            Description = string.Empty;
            IsCompleted = false;
            Start = new DateTime(2020, 1, 1, 0, 0, 0);
            IDcounter++;
        }

        private static int IDcounter = 0;

        private int itemID;

        public int ItemID
        {
            get
            {
                return itemID;
            }
            set
            {
                itemID = value;
            }
        }

        public string Priority
        {
            get
            {
                return (string)GetValue(PriorityProperty);
            }
            set
            {
                SetValue(PriorityProperty, value);
            }
        }

        public string Name
        {
            get
            {
                return (string)GetValue(NameProperty);
            }
            set
            {
                SetValue(NameProperty, value);
            }
        }

        public string Description
        {
            get
            {
                return (string)GetValue(DescriptionProperty);
            }
            set
            {
                SetValue(DescriptionProperty, value);
            }
        }

        public bool IsCompleted
        {
            get
            {
                return (bool)GetValue(IsCompletedProperty);
            }
            set
            {
                SetValue(IsCompletedProperty, value);
            }
        }

        private int itemType;

        public int ItemType //0 for task, 1 for appointment
        {
            get
            {
                return itemType;
            }
            set
            {
                itemType = value;
            }
        }

        public DateTime Start
        {
            get
            {
                return Convert.ToDateTime(GetValue(StartProperty));
            }
            set
            {
                SetValue(StartProperty, value);
            }
        }

        public virtual DateTime Stop
        {
            get;
            set;
        }

        public virtual string Attendees
        {
            get;
            set;
        }
    }
}
