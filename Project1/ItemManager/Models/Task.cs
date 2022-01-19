using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ItemManager.Models
{
    public class Task : Item
    {
        public Task()
        {
            ItemType = 0;
        }

        public override string ToString()
        {
            if(Priority == "!")
            {
                return $"   !   | {Name} - {Description} | Deadline: {Start:f}";
            }
            else if(Priority == "!!!")
            {
                return $"  !!!  | {Name} - {Description} | Deadline: {Start:f}";
            }
            else
            {
                return $" !!!!! | {Name} - {Description} | Deadline: {Start:f}";
            }
        }
    }
}
