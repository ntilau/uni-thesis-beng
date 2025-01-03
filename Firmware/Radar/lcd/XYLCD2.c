#include "XYLCD2.h"
#include <S1D13700_1.C>
#include <GRAPHICS.C>

CHAR CURS[]="#";
CHAR TEMP[50];
const int Persistenza=30;
signed long Iin=2048;
signed long Qin=2048;
signed long gain=50;
long Ioff=2071;
long Qoff=2087;
float BATTERY_LEVEL=0;
int n=0;
int MEDIA=15;
long BATTERY_INDICATOR;
long GAIN_INDICATOR;
long GAIN_TEXT_INDICATOR;
int1 RS232_FLAG=0;
int1 BATTERY_EXHAUSTED=0;
int1 DISPLAY_MODE=0; // 0=TEMPO, 1=FASORE;
signed long i,j,x[Persistenza+1],y[Persistenza+1];
signed long It[240],Qt[240];
unsigned long tempo=0;

#include "XYPLOT_FIRM.c"

#bit PLL_ENABLE=0xf9b.6

Batteria()
{
   set_adc_channel(1);

   BATTERY_LEVEL=read_adc();
   BATTERY_LEVEL=BATTERY_LEVEL/65;

   sprintf(temp,"%2.1fV",BATTERY_LEVEL);
   glcd_rect(282, 217, 318, 227, on, off);
   glcd_text57(282,217,temp , 1 , ON);

   BATTERY_INDICATOR=(BATTERY_LEVEL-10)*13;

/*   glcd_rect(245, 215, 275, 225, off, on);
   glcd_rect(246, 216, 246+BATTERY_INDICATOR, 224, on, on);
   glcd_rect(276, 215, 280, 225, on, off);
   glcd_rect(276, 218, 277, 222, on, on);
*/
   sprintf(temp,"Battery");
   glcd_text57(245,195,temp , 1 , ON);
   sprintf(temp,"Level");
   glcd_text57(265,205,temp , 1 , ON);

   set_adc_channel(0);
}


NULL_OFFSET()
{

   Ioff=Iin;
   Qoff=Qin;

   strcpy(temp,"!");
   glcd_text57(100,90,temp , 8 , ON);
   glcd_line(120, 70, 170 , 150, on);
   glcd_line(120, 70, 70 , 150, on);
   glcd_line(70, 150, 170 , 150, on);
   delay_ms(200);
   glcd_line(120, 70, 170, 150, off);
   glcd_line(120, 70, 70 , 150, off);
   glcd_line(70, 150, 170 , 150, off);
   glcd_text57(100,90,temp , 8 , Off);

}


#int_rda
Leggi()
{

   int Header,Imsb,Ilsb,Qmsb,Qlsb;

   header=fgetc(RS232);
   RS232_FLAG=1;
   if(Header==0xaa)
   {
      restart_wdt();

      Imsb=fgetc(rs232);
      Ilsb=fgetc(rs232);
      Qmsb=fgetc(rs232);
      Qlsb=fgetc(rs232);

      Iin=make16(Imsb,Ilsb);
      Qin=make16(Qmsb,Qlsb);

      tempo+=1;

      RS232_FLAG=1;
   }

}


SOPPRESSIONE()
{

   restart_wdt();

   strcpy(temp,"INHIBITED");
   glcd_text57(250,30,temp , 1 , off);

   fputc(0x63,RS232);
   fputc(0x0f,RS232);

   strcpy(temp,"TOGGLE");
   glcd_text57(260,30,temp , 1 , ON);
   delay_ms(1000);
   glcd_text57(260,30,temp , 1 , Off);

   Leggi();

   NULL_OFFSET();

}

Guadagno()
{

   sprintf(temp,"Gain=");
   glcd_text57(255,65,temp , 1 , ON);

   gain=read_adc();
   fputc(0x67,RS232);
   fputc(gain/4,RS232);

   GAIN_INDICATOR=(255-gain/4)/4;
   GAIN_TEXT_INDICATOR=(1024-gain)/4;
   GAIN_TEXT_INDICATOR=GAIN_TEXT_INDICATOR*0.29-8.13+40;

   //MEDIA=GAIN_TEXT_INDICATOR/4;

   glcd_rect(285, 65, 318, 75, on, off);
   sprintf(temp,"%lddB",GAIN_TEXT_INDICATOR);
   glcd_text57(285,65,temp , 1 , ON);

   glcd_rect(246, 81, 308, 81, on, off);
   glcd_rect(245, 80, 308, 82, off, on);
   glcd_rect(246, 81, 245+GAIN_INDICATOR, 81, on, on);

}


REDRAW_DISPLAY()
{
            tempo=0;
            glcd_fillScreen(off);
            griglia();
            Rett();
            Batteria();
            Guadagno();


            strcpy(temp,"Clutter");
            glcd_text57(255,10,temp , 1 , ON);
            strcpy(temp,"Suppression");
            glcd_text57(245,20,temp , 1 , ON);
            sprintf(temp,"RS232");
            glcd_text57(245,150,temp,1,on);
            sprintf(temp," Connection");
            glcd_text57(245,158,temp,1,on);

}

SWITCH_DISPLAY_MODE()
{

   if(DISPLAY_MODE)
      {
      DISPLAY_MODE=0;
      REDRAW_DISPLAY();
      }
   else
      {
      DISPLAY_MODE=1;
      REDRAW_DISPLAY();
      }
}

TEST_SOPPRESSIONE()
{

   if(!input(pin_b0))
   {

      SOPPRESSIONE();
      if(!input(pin_b0))
      {
         SWITCH_DISPLAY_MODE();
      }
   }
}


Test_Guadagno()
{

   if((read_adc()>gain+6)||(read_adc()<gain-6))
   {

      Guadagno();
   }

}

Reset_cmd()
{

   fputc(0x69,RS232);
   fputc(0x69,RS232);

   glcd_rect(15, 80, 225, 160, on, off);
   glcd_rect(15, 80, 225, 160, off, on);
   sprintf(temp,"RESETTING SYSTEM");
   glcd_text57(30,110,temp , 2 , ON);
   delay_ms(300);
   glcd_text57(30,110,temp , 2 , off);
   delay_ms(300);
   glcd_text57(30,110,temp , 2 , ON);
   delay_ms(300);
   glcd_text57(30,110,temp , 2 , off);
   glcd_rect(15, 80, 225, 160, off, off);

}

RS232_TEST()
{
   restart_wdt();

   while(!RS232_FLAG)
   {
      glcd_rect(260,173,318,180,on,off);
      delay_ms(200);
      sprintf(temp,"Missing!!");
      glcd_text57(260,173,temp,1,on);
      delay_ms(200);

   }

   if(RS232_FLAG)
   {
      glcd_rect(260,173,318,180,on,off);
      delay_ms(200);
      sprintf(temp,"Available");
      glcd_text57(260,173,temp,1,on);
      delay_ms(200);
   }

   RS232_FLAG=0;

}

Restart_measure()
{
   tempo=0;
   glcd_fillScreen(off);
   griglia();
   Rett();
   Batteria();
   Guadagno();


   strcpy(temp,"Clutter");
   glcd_text57(255,10,temp , 1 , ON);
   strcpy(temp,"Suppression");
   glcd_text57(245,20,temp , 1 , ON);
   sprintf(temp,"RS232");
   glcd_text57(245,150,temp,1,on);
   sprintf(temp," Connection");
   glcd_text57(245,158,temp,1,on);

}

void main()
{


   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
   setup_adc_ports(AN0_TO_AN1|VSS_VDD);
   setup_adc(ADC_CLOCK_INTERNAL|ADC_TAD_MUL_0);
   set_adc_channel(0);
   setup_oscillator(osc_8mhz);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128);

   glcd_init(ON);

   PLL_ENABLE=1;

   restart_wdt();
   glcd_fillScreen(off);


   griglia();
   Rett();

   strcpy(temp,"Clutter");
   glcd_text57(255,10,temp , 1 , ON);
   strcpy(temp,"Suppression");
   glcd_text57(245,20,temp , 1 , ON);
   sprintf(temp,"Battery");
   glcd_text57(245,195,temp , 1 , ON);
   sprintf(temp,"Level");
   glcd_text57(265,205,temp , 1 , ON);
   sprintf(temp,"RS232");
   glcd_text57(245,150,temp,1,on);
   sprintf(temp," Connection");
   glcd_text57(245,158,temp,1,on);
   sprintf(temp,"Gain=");
   glcd_text57(255,65,temp , 1 , ON);

   Reset_cmd();
   griglia();
   Rett();
   Guadagno();
   RS232_TEST();
   batteria();



   switch ( restart_cause() )
   {
      case 4:
      {
         strcpy(temp,"WTD Timeout");
         glcd_text57(250,115,temp , 1 , ON);
         delay_ms(1000);
         glcd_text57(250,115,temp , 1 , Off);
         break;
      }
      case 12:
      {
         strcpy(temp,"Power UP");
         glcd_text57(250,115,temp , 1 , ON);
         delay_ms(1000);
         glcd_text57(250,115,temp , 1 , Off);
         break;

      }
default:
      {
         strcpy(temp,"Unknown Event");
         glcd_text57(250,115,temp , 1 , ON);
         delay_ms(1000);
         glcd_text57(250,115,temp , 1 , Off);
         break;

      }
   }






/*while(0)
   {
      RS232_FLAG=0;

      restart_wdt();

      x[1]=(Iin-Ioff)/1+120;
      y[1]=(Qin-Qoff)/1+120;

      for(n=1;n<=MEDIA;++n)
      {
         x[1]+=x[n];
         y[1]+=y[n];
      }

      x[1]/=n;
      y[1]/=n;


      if((x[1]<10||y[1]<10||x[1]>230||y[1]>230)&&!CLUTTER_SUPPRESSOR_INHIBIT_FLAG)
      {
         NULL_OFFSET();
      }



      if(!CLUTTER_SUPPRESSOR_INHIBIT_FLAG)
      {
         if(Iin<50)
            SOPPRESSIONE();

         if(Qin<50)
            SOPPRESSIONE();

         if(Iin>4050)
            SOPPRESSIONE();

         if(Qin>4050)
            SOPPRESSIONE();

      }



      //if(CLUTTER_SUPPRESSOR_INHIBIT_FLAG)
      {
         if(x[1]<10)
         {
            x[1]=10;
         }
         if(y[1]<10)
         {
            y[1]=10;
         }
         if(x[1]>230)
         {
            x[1]=230;
         }
         if(y[1]>230)
         {
            y[1]=230;
         }
      }

      cursore(x[1],y[1],ON);

      delay_us(5000);

      for(i=(Persistenza);i>0;--i)
      {
         x[i]=x[i-1];
         y[i]=y[i-1];
      }

      cursore(x[persistenza],y[persistenza],OFF);

      TEST_SOPPRESSIONE();
      Guadagno();

      if(get_timer0()<500)
      {
         griglia();
         batteria();

      }

      if(RS232_FLAG==0)
         RS232_TEST();
   }


}*/

   tempo=0;
   while(1)
   {
      RS232_FLAG=0;

      restart_wdt();

      x[1]=(Iin-Ioff)/1+120;
      y[1]=(Qin-Qoff)/1+120;

      for(n=1;n<=MEDIA;++n)
      {
         x[1]+=x[n];
         y[1]+=y[n];
      }

      x[1]/=n;
      y[1]/=n;


      if((x[1]<10||y[1]<10||x[1]>230||y[1]>230))
      {
         NULL_OFFSET();
      }




         if(Iin<50)
            SOPPRESSIONE();

         if(Qin<50)
            SOPPRESSIONE();

         if(Iin>4050)
            SOPPRESSIONE();

         if(Qin>4050)
            SOPPRESSIONE();

         if(x[1]<10)
            x[1]=10;

         if(y[1]<10)
            y[1]=10;

         if(x[1]>230)
            x[1]=230;

         if(y[1]>230)
            y[1]=230;


         if(!DISPLAY_MODE)
            cursore(x[1],y[1],ON);
         else
         {
            cursore(tempo/15,x[1],ON);
            cursore(tempo/15,y[1],ON);
         }

         if(DISPLAY_MODE&&(tempo>230*15))
            REDRAW_DISPLAY();

         delay_us(9000);

         for(i=(Persistenza);i>0;--i)
         {
            x[i]=x[i-1];
            y[i]=y[i-1];
         }

         if(!DISPLAY_MODE)
            cursore(x[persistenza],y[persistenza],OFF);


         TEST_SOPPRESSIONE();
         test_Guadagno();


         if(RS232_FLAG==0)
            RS232_TEST();
      }


   }


