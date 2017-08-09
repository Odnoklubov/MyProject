using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO.Ports;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        public SerialPort mySerialPort;
        public bool isRec = false;
        public bool isRealTimeOn = false;
        private Message recMsg;

        List<Message> recMsgList;
        List<Message> uniqueIdLastMsgList;
        List<Message> uniqueIdMaxRssiMsgList;
        List<Message> uniqueIdMinRssiMsgList;

        int lodMsgsShowAmt = 0;
        bool isNewMsgRcvd = false;
        bool isComStarted = false;
        private bool isHideFirstTime = true;
        float lastTermTemp = 0;
        float lastNodeTemp = 0;
        int allMsgCount = 0;

        public Form1()
        {
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = true;
            InitializeComponent();
            string[] spNames = SerialPort.GetPortNames();
            comboBox1.Items.AddRange(spNames);
            Timer timer = new Timer();
            timer.Interval = (100); // 0.1 secs
            timer.Tick += new EventHandler(timer_Tick);
            timer.Start();
            recMsgList = new List<Message>();
            uniqueIdLastMsgList = new List<Message>();
            uniqueIdMaxRssiMsgList = new List<Message>();
            uniqueIdMinRssiMsgList = new List<Message>();
            comboBox1.DropDownStyle = ComboBoxStyle.DropDownList;
            recMsg = new Message();
            notifyIcon1.Text = "Temperature";

            //Write to file
            using (System.IO.StreamWriter file =
                new System.IO.StreamWriter(@"C:\Log.csv", true))
            {
                file.WriteLine("Start new session...");
            }
        }
        private void timer_Tick(object sender, EventArgs e)
        {
            if (recMsgList.Count > 0 && recMsgList.Count > lodMsgsShowAmt)
            {

                for (int j = lodMsgsShowAmt; j < recMsgList.Count; j++)
                {
                    DateTime dtOfCurMsg = DateTime.Now;
                    String Hour, Min, Sec;
                    Hour = dtOfCurMsg.Hour.ToString();
                    if (Hour.Length == 1)
                        Hour = "0" + Hour;
                    Min = dtOfCurMsg.Minute.ToString();
                    if (Min.Length == 1)
                        Min = "0" + Min;
                    Sec = dtOfCurMsg.Second.ToString();
                    if (Sec.Length == 1)
                        Sec = "0" + Sec;
                    listBox1.Items.Add(Hour + ":" + Min + ":" + Sec + " >> " + recMsgList[j].getMessage());

                    if (recMsg.getNodeId().Equals("Term"))
                    {
                        lastTermTemp = recMsg.getTemper();
                        label2.Text = recMsg.getTemper().ToString() + " C°";

                        label8.Text = Hour + ":" + Min + ":" + Sec;
                    }
                    if (recMsg.getNodeId().Equals("Node"))
                    {
                        label3.Text = recMsg.getTemper().ToString() + " C°";
                        label8.Text = Hour + ":" + Min + ":" + Sec;

                        lastNodeTemp = recMsg.getTemper();

                        allMsgCount++;

                        //Write to file
                        using (System.IO.StreamWriter file =
                            new System.IO.StreamWriter(@"D:\TempArchive\Log.csv", true))
                        {
                            file.WriteLine(allMsgCount.ToString() + ";" + Hour + ":" + Min + ":" + Sec + ";" 
                                + lastNodeTemp.ToString() + ";" + lastTermTemp.ToString());
                        }
                    }

                    this.Text = "Temperature " + label3.Text + " / " + label2.Text;
                    notifyIcon1.Text = "Temperature " + label3.Text + " / " + label2.Text;

                }
                listBox1.Refresh();
                lodMsgsShowAmt = recMsgList.Count;
            }
        }

        

        private void button1_Click(object sender, EventArgs e)
        {
            if (!isComStarted)
            {
                //sp = new SerialPort(comboBox1.Text.ToString(), 115200, Parity.None, 8, StopBits.One);
                try
                {
                    progressBar1.Minimum = 0;
                    progressBar1.Maximum = 5;
                    progressBar1.Step = 1;
                    mySerialPort = new SerialPort(comboBox1.Text.ToString());
                    progressBar1.PerformStep();
                    mySerialPort.BaudRate = 115200;
                    mySerialPort.Parity = Parity.None;
                    mySerialPort.StopBits = StopBits.One;
                    mySerialPort.DataBits = 8;
                    mySerialPort.Handshake = Handshake.None;
                    mySerialPort.RtsEnable = false;
                    mySerialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

                    progressBar1.PerformStep();
                
                    mySerialPort.Open();
                    isComStarted = true;
                    progressBar1.PerformStep();
                    button1.Text = "Stop";
                    label5.Text = "'" + comboBox1.Text.ToString() + "' Started...";
                    progressBar1.PerformStep();
                    comboBox1.Hide();
                    progressBar1.PerformStep();
                }
                catch (UnauthorizedAccessException)
                {
                    MessageBox.Show("COM Port " + comboBox1.Text.ToString() + " already opened by another program.");
                    progressBar1.Value = 0;
                }
                catch (ArgumentException)
                {
                    MessageBox.Show("COM Port field cannot be empty. Please fill COM Port Name.");
                    progressBar1.Value = 0;
                }
            }
            else
            {
                progressBar1.Step = -1;
                progressBar1.PerformStep();
                mySerialPort.Close();
                progressBar1.PerformStep();
                label5.Text = "";
                progressBar1.PerformStep();
                button1.Text = "Start";
                progressBar1.PerformStep();
                progressBar1.PerformStep();
                comboBox1.Show();
                isComStarted = false;
            }
        }
        void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            
            System.Threading.Thread.Sleep(10);
            try
            {
                SerialPort sp1 = (SerialPort)sender;
                recMsg.setMessage(sp1.ReadExisting());
                recMsg.parse();

                recMsgList.Add(recMsg);
                isNewMsgRcvd = true;
            }
            catch (TimeoutException) 
            {
                return;
            }
            catch (Exception)
            {
                return;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            listBox1.Items.Clear();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (isRealTimeOn)
            {
                isRealTimeOn = false;
                //button3.Text = "Real Time ON";
            }
            else 
            {
                isRealTimeOn = true;
                //button3.Text = "Real Time OFF";
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Rf Node Detecting program version 1.0.11. \nAuthor: Oleg Odnoklubov.");
        }

        private void button5_Click(object sender, EventArgs e)
        {
            listBox1.Items.Clear();
            /*listBox2.Items.Clear();
            listBox3.Items.Clear();
            listBox4.Items.Clear();
            listBox5.Items.Clear();*/
            recMsgList.Clear();
            uniqueIdLastMsgList.Clear();
            lodMsgsShowAmt = 0;
        }

        private void button6_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Form1_Resize(object sender, EventArgs e)
        {
            if (WindowState == FormWindowState.Minimized)
            {
                Hide();
                if (isHideFirstTime)
                {
                    notifyIcon1.BalloonTipTitle = "Программа была спрятана";
                    notifyIcon1.BalloonTipText = "Обратите внимание что программа была спрятана в трей и продолжает свою работу.";
                    notifyIcon1.ShowBalloonTip(500);
                    isHideFirstTime = false;
                }
            }
        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            Show();
            WindowState = FormWindowState.Normal;
        }



    }
}
