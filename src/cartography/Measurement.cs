using System;
using System.Collections.Generic;

namespace Cartography
{
    class Measurement
    {
        public List<int> measurements = new List<int>();

        public Measurement(string data)
        {
            foreach (var mess in data.Split(new[] { "," }, StringSplitOptions.None))
            {
                if (int.TryParse(mess, out int x))
                {
                    measurements.Add(x);
                }
            }
        }
    }
}
