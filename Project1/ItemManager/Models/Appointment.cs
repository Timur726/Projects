using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ItemManager.Models
{
    public class Appointment : Item
    {
        public Appointment()
        {
            Stop = new DateTime(2020, 1, 1, 0, 0, 0);
            Attendees = string.Empty;
            ItemType = 1;
        }

        public override DateTime Stop
        {
            get
            {
                return Convert.ToDateTime(GetValue(StopProperty));
            }
            set
            {
                SetValue(StopProperty, value);
            }
        }

        public override string Attendees
        {
            get
            {
                return (string)GetValue(AttendeesProperty);
            }
            set
            {
                SetValue(AttendeesProperty, value);
            }
        }

        public override string ToString()
        {
            if(Priority == "!")
            {
                return $"   !   | {Name} - {Description} | Start: {Start:f} - Stop: {Stop:f} | Attendees: {Attendees}";
            }
            else if (Priority == "!!!")
            {
                return $"  !!!  | {Name} - {Description} | Start: {Start:f} - Stop: {Stop:f} | Attendees: {Attendees}";
            }
            else
            {
                return $" !!!!! | {Name} - {Description} | Start: {Start:f} - Stop: {Stop:f} | Attendees: {Attendees}";
            }
        }
    }
}
