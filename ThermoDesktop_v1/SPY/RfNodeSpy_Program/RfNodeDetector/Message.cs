using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WindowsFormsApplication1
{
    public class Message
    {
        private string recMessage;
        private string nodeId;
        private float temper;
        private int rssiLevel;
        private DateTime dateTime;

        public Message()
        {
            dateTime = DateTime.Now;
        }
        public void setMessage(string msg)
        {
            recMessage = msg;
        }
        public void parse()
        {
            nodeId = recMessage.Substring(0, 4);
            temper = Convert.ToInt16(recMessage.Substring(7, 4), 16);
            rssiLevel = Convert.ToInt16(recMessage.Substring(5, 2), 16);
        }
        public string getNodeId()
        {
            return nodeId;
        }
        public float getTemper()
        {
            return temper/10;
        }
        public int getRssiLevel()
        {
            return rssiLevel;
        }
        public string getMessage()
        {
            return recMessage;
        }
        public DateTime getDateTime()
        {
            return dateTime;
        }
    }
}
