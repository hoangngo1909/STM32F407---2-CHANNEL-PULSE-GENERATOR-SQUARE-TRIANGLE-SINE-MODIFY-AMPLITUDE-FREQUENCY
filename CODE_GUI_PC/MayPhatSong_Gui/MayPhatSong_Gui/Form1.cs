using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ZedGraph;
using System.IO.Ports;

namespace MayPhatSong_Gui
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        int TickStart, intMode = 0;
        int intlen = 0;
        string[] baud = { "1200", "2400", "4800", "9600", "14400", "19200", "38400", "56000", "57600", "115200" };
        string[] databit = { "5", "6", "7", "8" };
        private void Form1_Load(object sender, EventArgs e)
        {
            GraphPane myPane = zed.GraphPane;
            myPane.Title.Text = "Wave Graph - 2 Channel";
            myPane.XAxis.Title.Text = "Time (s)";
            myPane.YAxis.Title.Text = "Amplitude (V)";
            RollingPointPairList list = new RollingPointPairList(60000);
            RollingPointPairList list1 = new RollingPointPairList(60000);

            LineItem curve = myPane.AddCurve("Channel1", list, Color.Red, SymbolType.None);
            LineItem curve1 = myPane.AddCurve("Channel2", list1, Color.Blue, SymbolType.None);

            myPane.XAxis.Scale.Min = 0;
            myPane.XAxis.Scale.Max = 30;
            myPane.XAxis.Scale.MinorStep = 1;
            myPane.XAxis.Scale.MajorStep = 5;
            zed.AxisChange();
            TickStart = Environment.TickCount;

            cbBaudrate.Items.AddRange(baud);
            cBoxDataBit.Items.AddRange(databit);
            cBoxParity.Items.AddRange(Enum.GetNames(typeof(Parity)));

            Mode1.Text = "0";
            Amplitude1.Text = "003";
            Frequency1.Text = "0000.1";
            Mode2.Text = "0";
            Amplitude2.Text = "003";
            Frequency2.Text = "0000.1";

            intMode = 1;
            receive = "0|0|000";
        }
        double intchannel1;
        double intchannel2;
        double intAmp1;
        double intAmp2;

        private void btConnect_Click(object sender, EventArgs e)
        {
            if (CbSecCom.Text == "")
            {
                MessageBox.Show("Vui long chon cong COM", "Thong Bao", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            if (cbBaudrate.Text == "")
            {
                MessageBox.Show("Vui long chon Baudrate", "Thong Bao", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (pbConnect.Text == "Connect")
            {
                try
                {
                    Com.PortName = CbSecCom.Text;
                    Com.BaudRate = int.Parse(cbBaudrate.Text);
                    Com.Open();
                    pbConnect.Text = "Disconnect";
                    pbConnect.BackColor = Color.Red;
                    pbConnect.ForeColor = Color.White;
                    CbSecCom.Enabled = false;
                    cbBaudrate.Enabled = false;
                    cBoxDataBit.Enabled = false;
                    cBoxParity.Enabled = false;
                    cBoxStopBit.Enabled = false;
                    progressBar1.Value = 100;
                }
                catch (Exception)
                {
                    MessageBox.Show("Không thể kết nối", "Thông Báo", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                while (Com.BytesToWrite > 0) { }
                Com.Dispose();
                Com.Close();
                pbConnect.Text = "Connect";
                pbConnect.BackColor = Color.Lime;
                pbConnect.ForeColor = Color.RoyalBlue;
                cbBaudrate.Enabled = true;
                CbSecCom.Enabled = true;
                cBoxDataBit.Enabled = true;
                cBoxParity.Enabled = true;
                cBoxStopBit.Enabled = true;
                progressBar1.Value = 0;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string s;
            if (pbConnect.Text == "Disconnect" && Mode1.Text != "" && Amplitude1.Text != "" && Frequency1.Text != "")
            {
                s = "|";
                switch (Amplitude1.TextLength)
                {
                    case 1:
                        Amplitude1.Text = "0" + "0" + Amplitude1.Text;
                        break;
                    case 2:
                        Amplitude1.Text = "0" + Amplitude1.Text;
                        break;
                    case 3:
                        Amplitude1.Text = Amplitude1.Text;
                        break;
                }
                switch (Frequency1.TextLength)
                {
                    case 1:
                        Frequency1.Text = "0" + "0" + "0" + "0" + "0" + Frequency1.Text;
                        break;
                    case 2:
                        Frequency1.Text = "0" + "0" + "0" + "0" + Frequency1.Text;
                        break;
                    case 3:
                        Frequency1.Text = "0" + "0" + "0" + Frequency1.Text;
                        break;
                    case 4:
                        Frequency1.Text = "0" + "0" + Frequency1.Text;
                        break;
                    case 5:
                        Frequency1.Text = "0" + Frequency1.Text;
                        break;
                    case 6:
                        Frequency1.Text = Frequency1.Text;
                        break;
                }
                if (Mode1.TextLength >= 2)
                {
                    Mode1.Text = "3";
                }
            }
            else
            {
                MessageBox.Show("Vui long nhap thong so dieu khien", "Thong Bao", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            
            if (pbConnect.Text == "Disconnect" && Mode2.Text != "" && Amplitude2.Text != "" && Frequency2.Text != "")
            {
                s = "|";
                switch (Amplitude2.TextLength)
                {
                    case 1:
                        Amplitude2.Text = "0" + "0" + Amplitude2.Text;
                        break;
                    case 2:
                        Amplitude2.Text = "0" + Amplitude2.Text;
                        break;
                    case 3:
                        Amplitude2.Text = Amplitude2.Text;
                        break;
                }
                switch (Frequency2.TextLength)
                {
                    case 1:
                        Frequency2.Text = "0" + "0" + "0" + "0" + "0" + Frequency2.Text;
                        break;
                    case 2:
                        Frequency2.Text = "0" + "0" + "0" + "0" + Frequency2.Text;
                        break;
                    case 3:
                        Frequency2.Text = "0" + "0" + "0" + Frequency2.Text;
                        break;
                    case 4:
                        Frequency2.Text = "0" + "0" + Frequency2.Text;
                        break;
                    case 5:
                        Frequency2.Text = "0" + Frequency2.Text;
                        break;
                    case 6:
                        Frequency2.Text = Frequency2.Text;
                        break;
                }
                if (Mode2.TextLength >= 2)
                {
                    Mode2.Text = "3";
                }

                Com.Write(Amplitude1.Text);
                Com.Write(s);
                Com.Write(Frequency1.Text);
                Com.Write(s);
                Com.Write(Mode1.Text);
                Com.Write(s);
                Com.Write(Amplitude2.Text);
                Com.Write(s);
                Com.Write(Frequency2.Text);
                Com.Write(s);
                Com.Write(Mode2.Text);
                Com.Write(s);
                int lensend = Mode1.TextLength + Amplitude1.TextLength + Frequency1.TextLength + Mode2.TextLength + Amplitude2.TextLength + Frequency2.TextLength + 6;
                for (int a = 0; a < 30 - lensend; a++)
                {
                    Com.Write(s);
                }
            }
            else
            {
                MessageBox.Show("Vui long nhap thong so dieu khien", "Thong Bao", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
        }

        string receive, sum;
        string[] List;
        private void Com_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            receive = Com.ReadExisting();
            
            List = receive.Split('|');
            if ((List[0] == "start") && (List[3] == "end"))
            {
                Draw(List[1], List[2]);
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();
            if (intlen != ports.Length)
            {
                intlen = ports.Length;
                CbSecCom.Items.Clear();
                for (int j = 0; j < intlen; j++)
                {
                    CbSecCom.Items.Add(ports[j]);
                }

            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (button2.Text == "SROLL")
            {
                intMode = 1;
                button2.Text = "COMPACT";
            }
            else
            {
                intMode = 0;
                button2.Text = "SROLL";
            }
        }

        private void Draw(string channel1, string channel2)
        {

            double.TryParse(channel1, out intchannel1);
            double.TryParse(channel2, out intchannel2);

            double.TryParse(Amplitude1.Text, out intAmp1);
            double.TryParse(Amplitude2.Text, out intAmp2);

            intchannel1 = intchannel1 * 3 / 4096;
            intchannel2 = intchannel2 * 3 / 4096;

            if (zed.GraphPane.CurveList.Count <= 0)
                return;

            LineItem curve = zed.GraphPane.CurveList[0] as LineItem;
            LineItem curve1 = zed.GraphPane.CurveList[1] as LineItem;
            if (curve == null)
                return;
            if (curve1 == null)
                return;

            IPointListEdit list = curve.Points as IPointListEdit;
            IPointListEdit list1 = curve1.Points as IPointListEdit;
            if (list == null)
                return;
            if (list1 == null)
                return;

            double time = (Environment.TickCount - TickStart) / 1000.0;

            list.Add(time, intchannel1);
            list1.Add(time, intchannel2);

            Scale xScale = zed.GraphPane.XAxis.Scale;
            if (time > xScale.Max - xScale.MajorStep)
            {
                if (intMode == 1)
                {
                    xScale.Max = time + xScale.MajorStep;
                    xScale.Min = xScale.Max - 20.0;
                }
                else
                {
                    xScale.Max = time + xScale.MajorStep;
                    xScale.Min = 0;
                }
            }

            zed.AxisChange();
            zed.Invalidate();
        }


    }
}
