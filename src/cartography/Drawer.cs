using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;

namespace Cartography
{
    class Drawer
    {
        public List<Measurement> measurements = new List<Measurement>();
        private readonly int hight = 1500; //y
        private readonly int width = 1000; //x
        private readonly int divX = 20;
        private readonly int divY = 20;
        private readonly float amplificationFactor = 0.7f;
        private readonly int zoneThreshold = 210;
        private readonly int zoneRadius = 8;

        public Drawer(List<Measurement> measurements)
        {
            this.measurements = measurements;
            Bitmap img = new Bitmap(width, hight);
            using (Graphics g = Graphics.FromImage(img)) { g.Clear(Color.White); }
            DrawMeasurementsPoints(img);
            DrawObstacleMap(img);
            DrawZones(img);
            DrawBackgroundSquares(img);
            img.Save("map.bmp", ImageFormat.Bmp);
        }

        private void DrawMeasurementsPoints(Bitmap bmp)
        {
            int counterMeasurements = 0;
            int counterAngle;
            for (int i = 0; i < 700; i += 100) // hight-400
            {
                counterAngle = 0;
                using (var graphics = Graphics.FromImage(bmp))
                {
                    Pen pen = new Pen(Color.Gray, 2);
                    for(int j = -80; j <= 90; j += 2)
                    {
                        if(measurements[counterMeasurements].measurements[counterAngle] > 200)
                        {
                            counterAngle++;
                            continue;
                        }
                        //float length = (float)Math.Log(measurements[counterMeasurements].measurements[counterAngle]*4.5)*100;
                        float length = measurements[counterMeasurements].measurements[counterAngle] * 10f; //ray length
                        float raySin = length * (float)Math.Sin(ToRad(j)); //ray x
                        float rayCos = length * (float)Math.Cos(ToRad(j)); //ray y
                        int x = (width / 2) + (int)raySin; //real ray x (from the middle)
                        int y = i + (int)rayCos;//real ray y (from the angle)
                        //graphics.DrawLine(pen, (width / 2) + 1, i - 2, x, y); //draws the ray line
                        if (x-5 > 0 && y-5 > 0 && x+5 < width && y+5 < hight)
                        {
                            int color = 0, count = 0;
                            for (int k = -5; k < 5; k++)
                            {
                                for (int l = -5; l < 5; l++)
                                {
                                    Color pixelColor = bmp.GetPixel(x+k, y+l);
                                    color += (pixelColor.R);
                                    count++;
                                }
                            }
                            color /= count;
                            color = (color - 40) < 0 ? 40 : color;
                            DrawSquare(bmp, x, y, 15, 15, Color.FromArgb(color - 40, color - 40, color - 40));
                            //bmp.SetPixel((width / 2) + (int)raySin, i + (int)rayCos, Color.Red);
                        }
                        else if (x > 0 && y > 0 && x < width && y < hight)
                        {
                            Color pixelColor = bmp.GetPixel(x, y);
                            int colorR = (pixelColor.R);
                            int colorG = (pixelColor.G);
                            int colorB = (pixelColor.B);
                            DrawSquare(bmp, x, y, 11, 11, Color.FromArgb(colorR - 40, colorG - 40, colorB - 40));
                            //bmp.SetPixel((width / 2) + (int)raySin, i + (int)rayCos, Color.Red);
                        }
                        counterAngle++;
                    }
                    /*
                    pen = new Pen(Color.Black, 2);
                    PointF start = new Point(150, i);
                    PointF point = new Point(180, i + 467);
                    PointF point2 = new Point(820, i + 467);
                    PointF end = new Point(850, i);
                    graphics.DrawBeziers(pen, new[] { start, point, end});
                    graphics.DrawBezier(pen, start.X, start.Y, point.X, point.Y, point2.X, point2.Y, end.X, end.Y);
                    //graphics.DrawLine();*/
                    
                    DrawSquare(bmp, (width / 2) - 4, i, 10, 10, Color.Red);
                    counterMeasurements++;
                }
            }
        }

        private void DrawObstacleMap(Bitmap bmp)
        {
            for (int i = 0; i < width / divX; i++)
            {
                for (int j = 0; j < hight / divY; j++)
                {
                    int x = i * divX;
                    int y = j * divY;
                    int posibility = 0;
                    int count = 0; 
                    for (int k = 0; k < divX-2; k++)
                    {
                        for (int l = 1; l < divY-2; l++)
                        {
                            Color pixelColor = bmp.GetPixel(x + 2 + k, y + 2 + l);
                            if(pixelColor.R < 255)
                            {
                                posibility -= (int)(amplificationFactor * ((float) pixelColor.R/255 * 100));
                            }
                            posibility += pixelColor.R;
                            count++;
                        }
                    }
                    posibility /= count;
                    DrawSquare(bmp, x + 2, y + 1, divY - 2, divX - 1, Color.FromArgb(posibility, posibility, posibility));
                }
            }
        }

        private void DrawZones(Bitmap bmp)
        {
            //green zones
            int x;
            int y;
            Color pixelColor;
            for (int i = -5; i < 5; i++)
            {
                x = (width / divX / 2 * divX) + (i * divX);
                y = 0;
                while (y+5 < hight && (pixelColor = bmp.GetPixel(x + 2, y + 5)).R >= zoneThreshold)
                {
                    DrawSquare(bmp, x + 2, y + 1, divY - 2, divX - 1, Color.FromArgb(0, 255, 0));
                    MoveLeft(x, y, bmp);
                    MoveRight(x, y, bmp);
                    y += divY;
                }
            }
        }

        private void MoveLeft(int x, int y, Bitmap bmp)
        {
            Color pixelColor;
            int left = x - divX; //one to left
            int max = x - zoneRadius * divX;
            while (left >= 0 && left >= max && (pixelColor = bmp.GetPixel(left + 2, y + 2)).R >= zoneThreshold)
            {
                DrawSquare(bmp, left + 2, y + 1, divY - 2, divX - 1, Color.FromArgb(0, 255, 0));
                left -= divX;
            }
        }

        private void MoveRight(int x, int y, Bitmap bmp)
        {
            Color pixelColor;
            int right = x + divX; //one to right
            int max = x + zoneRadius * divX;
            while (right < width && right <= max && (pixelColor = bmp.GetPixel(right + 2, y + 2)).R >= zoneThreshold)
            {
                DrawSquare(bmp, right + 2, y + 1, divY - 2, divX - 1, Color.FromArgb(0, 255, 0));
                right += divX;
            }
        }

        private void DrawBackgroundSquares(Bitmap bmp)
        {
            Pen pen = new Pen(Color.LightGray, 2);
            for (int i = 0; i < hight; i += divY)
            {
                for (int j = 0; j < width; j += divX)
                {
                    using (var graphics = Graphics.FromImage(bmp))
                    {
                        graphics.DrawLine(pen, i + 1, j + 1, i, hight);
                        graphics.DrawLine(pen, j, i, width, i);
                    }
                }
            }
            //last lines and first/last pixel
            using (var graphics = Graphics.FromImage(bmp))
            {
                graphics.DrawLine(pen, width-1, 0, width-1, hight-1);
                graphics.DrawLine(pen, 0, hight-1, width-1, hight-1);
                bmp.SetPixel(width - 1, hight - 1, Color.LightGray);
                bmp.SetPixel(0, 0, Color.LightGray);
            }
        }

        private void DrawSquare(Bitmap bmp, int x, int y, int squareHigh, int squareWidth, Color color)
        {
            Pen pen = new Pen(Color.LightGray, 2);
            using (var graphics = Graphics.FromImage(bmp))
            {
                graphics.FillRectangle(new SolidBrush(color), x, y, squareWidth, squareHigh);
            }
        }

        public void DrawLineInt(Bitmap bmp, int x1, int y1, int x2, int y2, Color color)
        {
            Pen pen = new Pen(color, 1);
            using (var graphics = Graphics.FromImage(bmp))
            {
                graphics.DrawLine(pen, x1, y1, x2, y2);
            }
        }

        private double ToRad(double angle)
       {
            return Math.PI * angle / 180.0;
        }
    }
}
