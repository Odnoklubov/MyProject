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
        private int msgNum;
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
            nodeId = recMessage.Substring(4, 8);
            msgNum = Convert.ToInt32(recMessage.Substring(0, 4), 16);
            rssiLevel = Convert.ToInt32(recMessage.Substring(12, 2), 16);
        }
        public string getNodeId()
        {
            return nodeId;
        }
        public int getMsgNum()
        {
            return msgNum;
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
