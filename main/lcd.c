#include "lcd.h"
#include "font.h"



_lcd_dev lcddev;

uint16_t BACK_COLOR, POINT_COLOR;   //背景色，画笔色


#ifndef SPI
//写8bit数据
void LCD_Writ_Bus(uint8_t da)   //串行数据写入
{		
	for(int i = 0; i < 8; i++)
	{
		if((da << i) & 0x80)
			LCD_SDI_1;
		else
			LCD_SDI_0;	

		LCD_SCK_0;
		LCD_SCK_1;		
	}
} 

//写16bit数据
void LCD_Write_Bus16(uint16_t da)   //串行数据写入
{		
	for(int i = 0; i < 16; i++)
	{
		if((da << i) & 0x8000)
			LCD_SDI_1;
		else
			LCD_SDI_0;	

		LCD_SCK_0;
		LCD_SCK_1;	
	}
} 

//读8bit数据
uint8_t LCD_Read_Bus()   
{
	uint8_t ret = 0;		
	for(int i = 0; i < 8; i++)
	{	
		LCD_SCK_1;	
		//vTaskDelay(1);
		LCD_SCK_0;
		ret <<= 1;
		if(LCD_SDO) ret |= 0x01;			
	}
	return ret;
} 


//写16bit数据
uint16_t LCD_Read_Bus16(void) 
{
	uint16_t ret = 0;		
	for(int i = 0; i < 16; i++)
	{	
		LCD_SCK_1;
		//vTaskDelay(1);
		LCD_SCK_0;	
		ret <<= 1;
		if(LCD_SDO) ret |= 0x01;		
	}
	return ret;
} 


void LCD_WR_DATA8(uint8_t da) //发送数据-8位参数
{
//	LCD_CS_0;
    LCD_DC_1;
	LCD_Writ_Bus(da);  
//	LCD_CS_1;
} 

//#define LCD_WR_DATA8 LCD_WR_DATA
void LCD_WR_DATA(uint16_t da)
{
//	LCD_CS_0;
	LCD_DC_1;
	LCD_Write_Bus16(da);
//	LCD_CS_1;
}	

void LCD_WR_REG(uint8_t da)	 
{	
//	LCD_CS_0;
    LCD_DC_0;
	LCD_Writ_Bus(da);
//	LCD_CS_1;
}
void LCD_Fast_WR_DATA(uint8_t* Color ,uint32_t len)
{
	LCD_DC_1;
	uint32_t j;
	for(j = 0; j < len ;j++)//刷新一行数据
        LCD_Writ_Bus(Color[j]);
	 
}  

void LCD_Fast_WR_DATA16(uint16_t* Color ,uint32_t len)
{
	LCD_DC_1;
	uint32_t i;
	for (i = 0;i < len; i++)
		LCD_Write_Bus16(Color[i]); 
}  

void LCD_Fast_WR_Color_DATA16(uint16_t Color ,uint32_t len)
{
	LCD_DC_1;
	uint32_t i;
	for (i = 0;i < len; i++)
	    LCD_Write_Bus16(Color); 	
	 
}  

void LCD_WR_REG_DATA(uint16_t reg,uint16_t da)
{
    LCD_WR_REG(reg);
	LCD_WR_DATA(da);
}
//读LCD数据
uint16_t LCD_RD_DATA(void)
{	
	return 0;
}
//读寄存器数据
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);
	return LCD_RD_DATA();
}

//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);  
}

//LCD写GRAM
void LCD_WriteRAM(uint16_t RGB_Code)
{
	LCD_WR_DATA(RGB_Code);  
}


//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
} 

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{ 
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x1);
	LCD_WR_DATA(x2);
	
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y1);
	LCD_WR_DATA(y2);

	LCD_WR_REG(0x2C);					 						 
}
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_CS_0;
    LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(Xpos);	 
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(Ypos);
	LCD_CS_1;
} 

//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试	   	   
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	uint16_t dirreg=0;
	uint16_t temp;  
	switch(dir)
	{
		case L2R_U2D://从左到右,从上到下
			regval|=(0<<7)|(0<<6)|(0<<5); 
			break;
		case L2R_D2U://从左到右,从下到上
			regval|=(1<<7)|(0<<6)|(0<<5); 
			break;
		case R2L_U2D://从右到左,从上到下
			regval|=(0<<7)|(1<<6)|(0<<5); 
			break;
		case R2L_D2U://从右到左,从下到上
			regval|=(1<<7)|(1<<6)|(0<<5); 
			break;	 
		case U2D_L2R://从上到下,从左到右
			regval|=(0<<7)|(0<<6)|(1<<5); 
			break;
		case U2D_R2L://从上到下,从右到左
			regval|=(0<<7)|(1<<6)|(1<<5); 
			break;
		case D2U_L2R://从下到上,从左到右
			regval|=(1<<7)|(0<<6)|(1<<5); 
			break;
		case D2U_R2L://从下到上,从右到左
			regval|=(1<<7)|(1<<6)|(1<<5); 
			break;	 
			  
		LCD_WR_REG_DATA(dirreg,regval);
 		if((regval&0X20)||lcddev.dir==1)
		{
			if(lcddev.width<lcddev.height)//交换X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}else  
		{
			if(lcddev.width>lcddev.height)//交换X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}  

		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(0);
		LCD_WR_DATA(lcddev.width-1);
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(0);
		LCD_WR_DATA(lcddev.height-1);  
  	}
}   

void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_CS_0;
	//LCD_SetCursor(x,y);		//设置光标位置 
	//LCD_WriteRAM_Prepare();	//开始写入GRAM
	Address_set(x,y,x,y);
	LCD_WR_DATA(POINT_COLOR); 
	LCD_CS_1;
}

//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{	
	LCD_CS_0;   
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(x);	 
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(y);
	
	LCD_WR_REG_DATA(lcddev.wramcmd,color);	
	LCD_CS_1;	 
}	

//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
	LCD_CS_0;
	if(dir==0)			//竖屏
	{
		lcddev.dir=0;	//竖屏
		lcddev.width=240;
		lcddev.height=320;
		
		lcddev.wramcmd=0X2C;
		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  	 
		
	}else 				//横屏
	{	  				
		lcddev.dir=1;	//横屏
		lcddev.width=320;
		lcddev.height=240;
		
		lcddev.wramcmd=0X2C;
		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  	 		
		
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
	LCD_CS_1;
}

//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
//68042,横屏时不支持窗口设置!! 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{    
	width=sx+width-1;
	height=sy+height-1;
	LCD_CS_0;
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(sx);	 
	LCD_WR_DATA(width);   
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(sy); 
	LCD_WR_DATA(height); 
	LCD_CS_1;
} 
//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r=0,g=0,b=0;
	uint16_t LCD_RAM;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回		   
	LCD_SetCursor(x,y);	    
	LCD_WR_REG(0X2E);//9341/6804/3510 发送读GRAM指令

	LCD_DC_1;
	LCD_RAM = LCD_RD_DATA();//第一次为假读
	LCD_RAM = LCD_RD_DATA();
	printf("point date4:0x%x\n",LCD_RAM);   

	if(LCD_RAM)r=0;							//dummy Read	     
 	r=LCD_RAM;  		  						//实际坐标颜色
 		  
	b = LCD_RAM; 
	g = r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
	g <<= 8;
	
	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));
}	

//LCD开启显示
void LCD_DisplayOn(void)
{	
	LCD_CS_0;				   
	LCD_WR_REG(0x29);
	LCD_CS_1;
}	 
//LCD关闭显示
void LCD_DisplayOff(void)
{	 
	LCD_CS_0;  
	LCD_WR_REG(0x28);
	LCD_CS_1;
}  

void LCD_delay(int t)
{
	vTaskDelay(t / portTICK_RATE_MS);
}

void Lcd_Init(void)
{
    gpio_set_direction(PIN_NUM_MISO, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_NUM_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_direction(25, GPIO_MODE_OUTPUT);
	//gpio_set_level(25,1);
	//gpio_set_level(PIN_NUM_MISO, 1);
    gpio_set_level(PIN_NUM_MOSI, 1);
    gpio_set_level(PIN_NUM_CLK, 1);
    //gpio_set_level(PIN_NUM_CS, 0);
    gpio_set_level(PIN_NUM_DC, 1);
    gpio_set_level(PIN_NUM_RST, 1);
	LCD_BL_1;

	LCD_CS_0;
	LCD_REST_0;
	LCD_delay(100);
	LCD_REST_1;
	LCD_delay(200);

    lcddev.dir = 0;	//竖屏
    lcddev.width = 240;
    lcddev.height = 320;
    lcddev.id = 0X9341;
    lcddev.wramcmd = 0X2C;
    lcddev.setxcmd = 0X2A;
    lcddev.setycmd = 0X2B;

    LCD_WR_REG(0xCF);  
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0XC1); 
    LCD_WR_DATA8(0X30); 

    LCD_WR_REG(0xED);  
    LCD_WR_DATA8(0x64); 
    LCD_WR_DATA8(0x03); 
    LCD_WR_DATA8(0X12); 
    LCD_WR_DATA8(0X81); 

    LCD_WR_REG(0xE8);  
    LCD_WR_DATA8(0x85); 
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x78); 

    LCD_WR_REG(0xCB);  
    LCD_WR_DATA8(0x39); 
    LCD_WR_DATA8(0x2C); 
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x34); 
    LCD_WR_DATA8(0x02); 
    
    LCD_WR_REG(0xF7);  
    LCD_WR_DATA8(0x20); 

    LCD_WR_REG(0xEA);  
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x00); 
    
    LCD_WR_REG(0xC0);    //Power control 
    LCD_WR_DATA8(0x23);   //VRH[5:0] 

    LCD_WR_REG(0xC1);    //Power control 
    LCD_WR_DATA8(0x10);   //SAP[2:0];BT[3:0] 

    LCD_WR_REG(0xC5);    //VCM control 
    LCD_WR_DATA8(0x3e); //对比度调节
    LCD_WR_DATA8(0x28); 

    LCD_WR_REG(0xC7);    //VCM control2 
    LCD_WR_DATA8(0x86);  //--

    LCD_WR_REG(0x36);    // Memory Access Control 
    LCD_WR_DATA8(0x48); //	   //48 68竖屏//28 E8 横屏

    LCD_WR_REG(0x3A);    
    LCD_WR_DATA8(0x55); 

    LCD_WR_REG(0xB1);    
    LCD_WR_DATA8(0x00);  
    LCD_WR_DATA8(0x18); 

    LCD_WR_REG(0xB6);    // Display Function Control 
    LCD_WR_DATA8(0x08); 
    LCD_WR_DATA8(0x82);
    LCD_WR_DATA8(0x27);  
    
    LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
    LCD_WR_DATA8(0x00); 
    
    LCD_WR_REG(0x26);    //Gamma curve selected 
    LCD_WR_DATA8(0x01); 

    LCD_WR_REG(0xE0);    //Set Gamma 
    LCD_WR_DATA8(0x0F); 
    LCD_WR_DATA8(0x31); 
    LCD_WR_DATA8(0x2B); 
    LCD_WR_DATA8(0x0C); 
    LCD_WR_DATA8(0x0E); 
    LCD_WR_DATA8(0x08); 
    LCD_WR_DATA8(0x4E); 
    LCD_WR_DATA8(0xF1); 
    LCD_WR_DATA8(0x37); 
    LCD_WR_DATA8(0x07); 
    LCD_WR_DATA8(0x10); 
    LCD_WR_DATA8(0x03); 
    LCD_WR_DATA8(0x0E); 
    LCD_WR_DATA8(0x09); 
    LCD_WR_DATA8(0x00); 

    LCD_WR_REG(0XE1);    //Set Gamma 
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x0E); 
    LCD_WR_DATA8(0x14); 
    LCD_WR_DATA8(0x03); 
    LCD_WR_DATA8(0x11); 
    LCD_WR_DATA8(0x07); 
    LCD_WR_DATA8(0x31); 
    LCD_WR_DATA8(0xC1); 
    LCD_WR_DATA8(0x48); 
    LCD_WR_DATA8(0x08); 
    LCD_WR_DATA8(0x0F); 
    LCD_WR_DATA8(0x0C); 
    LCD_WR_DATA8(0x31); 
    LCD_WR_DATA8(0x36); 
    LCD_WR_DATA8(0x0F); 

    LCD_WR_REG(0x2B); 
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x01);
    LCD_WR_DATA8(0x3f);
    
    LCD_WR_REG(0x2A); 
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xef);

    LCD_WR_REG(0x11);    //Exit Sleep 
    
    LCD_delay(120); 

    LCD_WR_REG(0x29);    //Display on 
    LCD_WR_REG(0x2c);
	printf("light on\n");
	gpio_set_level(25,1);
    LCD_BL_1;
	LCD_CS_1;
    LCD_Display_Dir(0);
	printf("clear\n");
	LCD_Clear(WHITE); //清屏 
}
//清屏函数
//Color:要清屏的填充色

void LCD_Clear(uint16_t Color)
{
	LCD_CS_0;
	Address_set(0,0,lcddev.width - 1,lcddev.height - 1);
	LCD_Fast_WR_Color_DATA16(Color,(lcddev.width)*(lcddev.height)); 	 
	LCD_CS_1;
}

// //画点
// //POINT_COLOR:此点的颜色
// static void LCD_DrawPoint(uint16_t x,uint16_t y)
// {
// 	Address_set(x,y,x,y);//设置光标位置 
// 	LCD_WR_DATA(POINT_COLOR); 	    
// } 	 
//画一个大点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint_big(uint16_t x,uint16_t y)
{
	LCD_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);
} 
//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{     
	LCD_CS_0;      
	Address_set(xsta,ysta,xend,yend);      //设置光标位置 
	LCD_Fast_WR_Color_DATA16(color,(xend-xsta+1) * (yend-ysta+1));	    
	LCD_CS_1;				  	    
} 

//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill8(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint8_t *color)
{  
	LCD_CS_0;
	Address_set(sx,sy,ex,ey);
	LCD_Fast_WR_DATA(color,2*(ex-sx+1) * (ey-sy+1));
	LCD_CS_1;
}
//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
	LCD_CS_0;
	Address_set(sx,sy,ex,ey);
	LCD_Fast_WR_DATA16(color,(ex-sx+1) * (ey-sy+1));
	LCD_CS_1;	  
}  
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}
//画矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_Fill(x1,y1,x2,y1,POINT_COLOR);
	LCD_Fill(x1,y1,x1,y2,POINT_COLOR);
	LCD_Fill(x1,y2,x2,y2,POINT_COLOR);
	LCD_Fill(x2,y1,x2,y2,POINT_COLOR);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             //5
 		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-a,y0+b);             //1       
 		LCD_DrawPoint(x0-b,y0+a);             
		LCD_DrawPoint(x0-a,y0-b);             //2             
  		LCD_DrawPoint(x0-b,y0-a);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"
//mode:叠加方式(1)还是非叠加方式(0)
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"

//mode:叠加方式(1)还是非叠加方式(0)

void LCD_ShowChar(uint16_t x,uint16_t y,char num,uint8_t size,uint8_t mode)
{  							  
    uint8_t temp = 0,t1,t,xwidth;
	uint16_t y0=y;
	uint16_t colortemp=POINT_COLOR;   
	uint16_t SPI_LCD_RAM[16*16];
	xwidth = size/2;   			     
	//设置窗口		  
	//Address_set(x,y,x+size/2-1,y+size-1); 
	num = num - ' ';//得到偏移后的值
	if(!mode) //非叠加方式
	{
	    for(t=0;t < size;t++)
	    {   
			if(size == 12)temp=asc2_1206[(uint8_t)num][t];  //调用1206字体
			else temp=asc2_1608[(uint8_t)num][t];		 //调用1608字体 	                          
	        for(t1 = 0;t1 < 8;t1++)
			{			    
		        if(temp&0x01)POINT_COLOR = colortemp;
				else POINT_COLOR = BACK_COLOR;
				SPI_LCD_RAM[t*xwidth + t1] = POINT_COLOR;	//先存起来再显示
				//LCD_DrawPoint(x,y);	
				temp>>=1;
				y++;
				if(y>=lcddev.height){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }    
	}else//叠加方式
	{
	    for(t=0;t<size;t++)
	    {   
			if(size == 12)temp=asc2_1206[(uint8_t)num][t];  //调用1206字体
			else temp=asc2_1608[(uint8_t)num][t];		 //调用1608字体 	                          
	        for(t1=0;t1 < 8;t1++)
			{			    
		        if(temp&0x01)//从左到右 逐列扫描
				{
					//LCD_DrawPoint(x,y); 
					SPI_LCD_RAM[t*xwidth + t1] = POINT_COLOR;	//先存起来再显示
				}
				else SPI_LCD_RAM[t*xwidth + t1] = BACK_COLOR;
				
				temp>>=1;
				y++;
				if(y>=lcddev.height){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }     
	}
	LCD_Color_Fill(x,y,x+xwidth-1,y+size-1,SPI_LCD_RAM);
	POINT_COLOR=colortemp;	    	   	 	  
} 

//m^n函数
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//color:颜色
//num:数值(0~4294967295);	
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	num=(uint16_t)num;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+8*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+8*t,y,temp+48,size,0); 
	}
} 
//显示2个数字
//x,y:起点坐标
//num:数值(0~99);	 
void LCD_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+8*t,y,temp+'0',size,0); 
	}
} 

//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p)
{         
	uint16_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}
//在指定位置显示一个汉字(32*32大小)
//dcolor为内容颜色，gbcolor为背静颜色
void showhanzi(unsigned int x,unsigned int y,char *p,uint8_t size)	
{  
	unsigned char i,j;
	unsigned char *temp = (uint8_t *)p;  
	uint16_t SPI_LCD_RAM[32*32];
	uint16_t cnt = 32 * mypow(size/16,2); 	
	for(j=0;j<cnt;j++)
	{
		for(i=0;i<8;i++)
		{ 		     
		 	if((*temp&(1<<i))!=0)	
				SPI_LCD_RAM[j*8 + i] = POINT_COLOR;
			else
				SPI_LCD_RAM[j*8 + i] = BACK_COLOR; 
		}
		temp++;
	 }
	 LCD_Color_Fill(x,y,x + size - 1,y + size -1,SPI_LCD_RAM);
}

void showimage(uint16_t x,uint16_t y) //显示40*40图片
{  
	uint16_t i,j,k;
	uint16_t da;
	uint16_t SPI_LCD_RAM[40*40];
	k=0;
	for(i=0;i<40;i++)
	{	
		for(j=0;j<40;j++)
		{
			da=qqimage[k*2+1];
			da<<=8;
			da|=qqimage[k*2]; 
			SPI_LCD_RAM[i*40 + j] = da;					
			k++;  			
		}
	}
	LCD_Color_Fill(x,y,x+39,y+39,SPI_LCD_RAM);
}

#else //硬件SPI驱动

extern spi_device_handle_t spi;
//uint16_t SPI_LCD_RAM[320*240];//显示缓存
//Send a command to the LCD. Uses spi_device_transmit, which waits until the transfer is complete.
void lcd_cmd(const uint8_t cmd) 
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}
//Send data to the LCD. Uses spi_device_transmit, which waits until the transfer is complete.
void lcd_data(const uint8_t *data, int len) 
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}
void lcd_spi_pre_transfer_callback(spi_transaction_t *t) 
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void LCD_WR_DATA8(const uint8_t da) //发送数据-8位参数
{
	lcd_data(&da, 1);
} 

//#define LCD_WR_DATA8 LCD_WR_DATA
void LCD_WR_DATA(uint16_t da)
{
	uint8_t dat[2];
	dat[0] = da >>8;
	dat[1] = da;
	lcd_data(dat, 2);
}	

void LCD_WR_REG(uint8_t da)	 
{	
	lcd_cmd(da);
}

#define BUF_LEN 512

void LCD_Fast_WR_DATA(uint8_t* Color ,uint32_t len)
{
	uint32_t j,k;
	uint8_t *SPI_LCD_RAM = (uint8_t *)malloc(len);
	k = len / BUF_LEN;
	for(j = 0; j < k ;j++)//刷新一行数据
		lcd_data(Color + BUF_LEN*j, BUF_LEN);	

	k = len % BUF_LEN;
	if(k)	 
		lcd_data(SPI_LCD_RAM + BUF_LEN*j, k);//最后写入不满32字节的包	 
}  

void LCD_Fast_WR_DATA16(uint16_t* Color ,uint32_t len)
{
	uint32_t i,k;
	uint8_t *SPI_LCD_RAM = (uint8_t *)malloc(2 * len);	
	k = len*2 / BUF_LEN;
	for (i = 0;i < len; i++)
	{
		SPI_LCD_RAM[ 2*i ] = Color[i] >> 8;//高位
		SPI_LCD_RAM[ 2*i + 1 ] = Color[i]; //低位
	}
	for(i = 0; i < k ;i++)//刷新一行数据
		lcd_data(SPI_LCD_RAM + BUF_LEN*i, BUF_LEN);

	k = len*2 % BUF_LEN;
	if(k)	 
		lcd_data(SPI_LCD_RAM + BUF_LEN*i, k);//最后写入不满32字节的包

	free(SPI_LCD_RAM);		 
}  

void LCD_Fast_WR_Color_DATA16(uint16_t Color ,uint32_t len)
{
	uint32_t i,k;
	uint8_t *SPI_LCD_RAM = (uint8_t *)malloc(2 * len);	
	k = len*2 / BUF_LEN;
	for (i = 0;i < len; i++)
	{
		SPI_LCD_RAM[ 2*i ] = Color >> 8;//高位
		SPI_LCD_RAM[ 2*i + 1 ] = Color; //低位
	}	
	for(i = 0; i < k ;i++)//刷新一行数据
		lcd_data(SPI_LCD_RAM + BUF_LEN*i, BUF_LEN);

	k = len*2 % BUF_LEN;
	if(k)	 
		lcd_data(SPI_LCD_RAM + BUF_LEN*i, k);//最后写入不满32字节的包

	free(SPI_LCD_RAM);	 
} 

void LCD_WR_REG_DATA(uint16_t reg,uint16_t da)
{
    LCD_WR_REG(reg);
	LCD_WR_DATA(da);
}
//读LCD数据
uint16_t LCD_RD_DATA(void)
{	
	return 0;
}
//读寄存器数据
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);
	return LCD_RD_DATA();
}

//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);  
}

//LCD写GRAM
void LCD_WriteRAM(uint16_t RGB_Code)
{
	LCD_WR_DATA(RGB_Code);  
}


//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
} 

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{ 
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x1);
	LCD_WR_DATA(x2);
	
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y1);
	LCD_WR_DATA(y2);

	LCD_WR_REG(0x2C);					 						 
}
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
    LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(Xpos);	 
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(Ypos);
} 

//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试	   	   
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	uint16_t dirreg=0;
	uint16_t temp;  
	switch(dir)
	{
		case L2R_U2D://从左到右,从上到下
			regval|=(0<<7)|(0<<6)|(0<<5); 
			break;
		case L2R_D2U://从左到右,从下到上
			regval|=(1<<7)|(0<<6)|(0<<5); 
			break;
		case R2L_U2D://从右到左,从上到下
			regval|=(0<<7)|(1<<6)|(0<<5); 
			break;
		case R2L_D2U://从右到左,从下到上
			regval|=(1<<7)|(1<<6)|(0<<5); 
			break;	 
		case U2D_L2R://从上到下,从左到右
			regval|=(0<<7)|(0<<6)|(1<<5); 
			break;
		case U2D_R2L://从上到下,从右到左
			regval|=(0<<7)|(1<<6)|(1<<5); 
			break;
		case D2U_L2R://从下到上,从左到右
			regval|=(1<<7)|(0<<6)|(1<<5); 
			break;
		case D2U_R2L://从下到上,从右到左
			regval|=(1<<7)|(1<<6)|(1<<5); 
			break;	 
			  
		LCD_WR_REG_DATA(dirreg,regval);
 		if((regval&0X20)||lcddev.dir==1)
		{
			if(lcddev.width<lcddev.height)//交换X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}else  
		{
			if(lcddev.width>lcddev.height)//交换X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}  

		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(0);
		LCD_WR_DATA(lcddev.width-1);
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(0);
		LCD_WR_DATA(lcddev.height-1);  
  	}
}   

void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	//LCD_SetCursor(x,y);		//设置光标位置 
	//LCD_WriteRAM_Prepare();	//开始写入GRAM
	Address_set(x,y,x,y);
	LCD_WR_DATA(POINT_COLOR); 
}

//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(x);	 
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(y);
	
	LCD_WR_REG_DATA(lcddev.wramcmd,color);		
}	

//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
	if(dir==0)			//竖屏
	{
		lcddev.dir=0;	//竖屏
		lcddev.width=240;
		lcddev.height=320;
		
		lcddev.wramcmd=0X2C;
		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  	 
		
	}else 				//横屏
	{	  				
		lcddev.dir=1;	//横屏
		lcddev.width=320;
		lcddev.height=240;
		
		lcddev.wramcmd=0X2C;
		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  	 		
		
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}

//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
//68042,横屏时不支持窗口设置!! 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{    
	width=sx+width-1;
	height=sy+height-1;
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(sx);	 
	LCD_WR_DATA(width);   
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(sy); 
	LCD_WR_DATA(height); 
} 
//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r=0,g=0,b=0;
	uint16_t LCD_RAM;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回		   
	LCD_SetCursor(x,y);	    
	LCD_WR_REG(0X2E);//9341/6804/3510 发送读GRAM指令

	LCD_RAM = LCD_RD_DATA();//第一次为假读
	LCD_RAM = LCD_RD_DATA();
	printf("point date4:0x%x\n",LCD_RAM);   

	if(LCD_RAM)r=0;							//dummy Read	     
 	r=LCD_RAM;  		  						//实际坐标颜色
 		  
	b = LCD_RAM; 
	g = r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
	g <<= 8;
	
	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));
}	

//LCD开启显示
void LCD_DisplayOn(void)
{	
	LCD_WR_REG(0x29);
}	 
//LCD关闭显示
void LCD_DisplayOff(void)
{	 
	LCD_WR_REG(0x28);
}  

void LCD_delay(int t)
{
	vTaskDelay(t / portTICK_RATE_MS);
}


spi_device_handle_t spi;
void Lcd_Init(void)
{  
	esp_err_t ret; 
	gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    gpio_set_level(PIN_NUM_CS, 1);
    gpio_set_level(PIN_NUM_DC, 1);
    gpio_set_level(PIN_NUM_RST, 1);
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
		//.max_transfer_sz = 320 * 240 * 2,
    }; 
    spi_device_interface_config_t devcfg={
		.clock_speed_hz = 25000000,				// Initial clock out at 8 MHz
		.mode = 0,								// SPI mode 0
		.spics_io_num = PIN_NUM_CS,				// set SPI CS pin
        .queue_size=BUF_LEN,                    //We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    assert(ret==ESP_OK);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    assert(ret==ESP_OK);
    
	LCD_BL_0;

    // LCD_REST_0;
	// LCD_delay(100);
	// LCD_REST_1;
	// LCD_delay(200);

    lcddev.dir = 0;	//竖屏
    lcddev.width = 240;
    lcddev.height = 320;
    lcddev.id = 0X9341;
    lcddev.wramcmd = 0X2C;
    lcddev.setxcmd = 0X2A;
    lcddev.setycmd = 0X2B;

    LCD_WR_REG(0xCF);  
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0XC1); 
    LCD_WR_DATA8(0X30); 

    LCD_WR_REG(0xED);  
    LCD_WR_DATA8(0x64); 
    LCD_WR_DATA8(0x03); 
    LCD_WR_DATA8(0X12); 
    LCD_WR_DATA8(0X81); 

    LCD_WR_REG(0xE8);  
    LCD_WR_DATA8(0x85); 
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x78); 

    LCD_WR_REG(0xCB);  
    LCD_WR_DATA8(0x39); 
    LCD_WR_DATA8(0x2C); 
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x34); 
    LCD_WR_DATA8(0x02); 
    
    LCD_WR_REG(0xF7);  
    LCD_WR_DATA8(0x20); 

    LCD_WR_REG(0xEA);  
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x00); 
    
    LCD_WR_REG(0xC0);    //Power control 
    LCD_WR_DATA8(0x23);   //VRH[5:0] 

    LCD_WR_REG(0xC1);    //Power control 
    LCD_WR_DATA8(0x10);   //SAP[2:0];BT[3:0] 

    LCD_WR_REG(0xC5);    //VCM control 
    LCD_WR_DATA8(0x3e); //对比度调节
    LCD_WR_DATA8(0x28); 

    LCD_WR_REG(0xC7);    //VCM control2 
    LCD_WR_DATA8(0x86);  //--

    LCD_WR_REG(0x36);    // Memory Access Control 
    LCD_WR_DATA8(0x48); //	   //48 68竖屏//28 E8 横屏

    LCD_WR_REG(0x3A);    
    LCD_WR_DATA8(0x55); 

    LCD_WR_REG(0xB1);    
    LCD_WR_DATA8(0x00);  
    LCD_WR_DATA8(0x18); 

    LCD_WR_REG(0xB6);    // Display Function Control 
    LCD_WR_DATA8(0x08); 
    LCD_WR_DATA8(0x82);
    LCD_WR_DATA8(0x27);  
    
    LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
    LCD_WR_DATA8(0x00); 
    
    LCD_WR_REG(0x26);    //Gamma curve selected 
    LCD_WR_DATA8(0x01); 

    LCD_WR_REG(0xE0);    //Set Gamma 
    LCD_WR_DATA8(0x0F); 
    LCD_WR_DATA8(0x31); 
    LCD_WR_DATA8(0x2B); 
    LCD_WR_DATA8(0x0C); 
    LCD_WR_DATA8(0x0E); 
    LCD_WR_DATA8(0x08); 
    LCD_WR_DATA8(0x4E); 
    LCD_WR_DATA8(0xF1); 
    LCD_WR_DATA8(0x37); 
    LCD_WR_DATA8(0x07); 
    LCD_WR_DATA8(0x10); 
    LCD_WR_DATA8(0x03); 
    LCD_WR_DATA8(0x0E); 
    LCD_WR_DATA8(0x09); 
    LCD_WR_DATA8(0x00); 

    LCD_WR_REG(0XE1);    //Set Gamma 
    LCD_WR_DATA8(0x00); 
    LCD_WR_DATA8(0x0E); 
    LCD_WR_DATA8(0x14); 
    LCD_WR_DATA8(0x03); 
    LCD_WR_DATA8(0x11); 
    LCD_WR_DATA8(0x07); 
    LCD_WR_DATA8(0x31); 
    LCD_WR_DATA8(0xC1); 
    LCD_WR_DATA8(0x48); 
    LCD_WR_DATA8(0x08); 
    LCD_WR_DATA8(0x0F); 
    LCD_WR_DATA8(0x0C); 
    LCD_WR_DATA8(0x31); 
    LCD_WR_DATA8(0x36); 
    LCD_WR_DATA8(0x0F); 

    LCD_WR_REG(0x2B); 
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x01);
    LCD_WR_DATA8(0x3f);
    
    LCD_WR_REG(0x2A); 
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0xef);

    LCD_WR_REG(0x11);    //Exit Sleep 
    
    LCD_delay(120); 

    LCD_WR_REG(0x29);    //Display on 
    LCD_WR_REG(0x2c);
    LCD_BL_1;
	LCD_Display_Dir(0);
}
//清屏函数
//Color:要清屏的填充色

void LCD_Clear(uint16_t Color)
{
	Address_set(0,0,lcddev.width - 1,lcddev.height - 1);
	LCD_Fast_WR_Color_DATA16(Color,(lcddev.width)*(lcddev.height)); 	
}
 
//画一个大点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint_big(uint16_t x,uint16_t y)
{
	LCD_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);
} 
//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{   
	Address_set(xsta,ysta,xend,yend);      //设置光标位置 
	LCD_Fast_WR_Color_DATA16(color,(xend-xsta+1) * (yend-ysta+1));    				  	    
} 

//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill8(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint8_t *color)
{  
	Address_set(sx,sy,ex,ey);
	LCD_Fast_WR_DATA(color,2*(ex-sx+1) * (ey-sy+1));
}
//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
	Address_set(sx,sy,ex,ey);
	LCD_Fast_WR_DATA16(color,(ex-sx+1) * (ey-sy+1));	
}  
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}
//画矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_Fill(x1,y1,x2,y1,POINT_COLOR);
	LCD_Fill(x1,y1,x1,y2,POINT_COLOR);
	LCD_Fill(x1,y2,x2,y2,POINT_COLOR);
	LCD_Fill(x2,y1,x2,y2,POINT_COLOR);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             //5
 		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-a,y0+b);             //1       
 		LCD_DrawPoint(x0-b,y0+a);             
		LCD_DrawPoint(x0-a,y0-b);             //2             
  		LCD_DrawPoint(x0-b,y0-a);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"
//mode:叠加方式(1)还是非叠加方式(0)
//在指定位置显示一个字符

//num:要显示的字符:" "--->"~"

//mode:叠加方式(1)还是非叠加方式(0)

void LCD_ShowChar(uint16_t x,uint16_t y,char num,uint8_t size,uint8_t mode)
{  							  
    uint8_t temp = 0,t1,t,xwidth;
	uint16_t y0=y;
	uint16_t colortemp=POINT_COLOR;   
	uint16_t SPI_LCD_RAM[16*16];
	xwidth = size/2;   			     
	//设置窗口		  
	//Address_set(x,y,x+size/2-1,y+size-1); 
	num = num - ' ';//得到偏移后的值
	if(!mode) //非叠加方式
	{
	    for(t=0;t < size;t++)
	    {   
			if(size == 12)temp=asc2_1206[(uint8_t)num][t];  //调用1206字体
			else temp=asc2_1608[(uint8_t)num][t];		 //调用1608字体 	                          
	        for(t1 = 0;t1 < 8;t1++)
			{			    
		        if(temp&0x01)POINT_COLOR = colortemp;
				else POINT_COLOR = BACK_COLOR;
				SPI_LCD_RAM[t*xwidth + t1] = POINT_COLOR;	//先存起来再显示
				//LCD_DrawPoint(x,y);	
				temp>>=1;
				y++;
				if(y>=lcddev.height){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }    
	}else//叠加方式
	{
	    for(t=0;t<size;t++)
	    {   
			if(size == 12)temp=asc2_1206[(uint8_t)num][t];  //调用1206字体
			else temp=asc2_1608[(uint8_t)num][t];		 //调用1608字体 	                          
	        for(t1=0;t1 < 8;t1++)
			{			    
		        if(temp&0x01)//从左到右 逐列扫描
				{
					//LCD_DrawPoint(x,y); 
					SPI_LCD_RAM[t*xwidth + t1] = POINT_COLOR;	//先存起来再显示
				}
				else SPI_LCD_RAM[t*xwidth + t1] = BACK_COLOR;
				
				temp>>=1;
				y++;
				if(y>=lcddev.height){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }     
	}
	LCD_Color_Fill(x,y,x+xwidth-1,y+size-1,SPI_LCD_RAM);
	POINT_COLOR=colortemp;	    	   	 	  
} 

//m^n函数
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//color:颜色
//num:数值(0~4294967295);	
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	num=(uint16_t)num;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+8*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+8*t,y,temp+48,size,0); 
	}
} 
//显示2个数字
//x,y:起点坐标
//num:数值(0~99);	 
void LCD_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+8*t,y,temp+'0',size,0); 
	}
} 

//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p)
{         
	uint16_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}
//在指定位置显示一个汉字(32*32大小)
//dcolor为内容颜色，gbcolor为背静颜色
void showhanzi(unsigned int x,unsigned int y,char *p,uint8_t size)	
{  
	unsigned char i,j;
	unsigned char *temp = (uint8_t *)p;  
	uint16_t SPI_LCD_RAM[32*32];
	uint16_t cnt = 32 * mypow(size/16,2); 	
	for(j=0;j<cnt;j++)
	{
		for(i=0;i<8;i++)
		{ 		     
		 	if((*temp&(1<<i))!=0)	
				SPI_LCD_RAM[j*8 + i] = POINT_COLOR;
			else
				SPI_LCD_RAM[j*8 + i] = BACK_COLOR; 
		}
		temp++;
	 }
	 LCD_Color_Fill(x,y,x + size - 1,y + size -1,SPI_LCD_RAM);
}

void showimage(uint16_t x,uint16_t y) //显示40*40图片
{  
	uint16_t i,j,k;
	uint16_t da;
	uint16_t SPI_LCD_RAM[40*40];
	k=0;
	for(i=0;i<40;i++)
	{	
		for(j=0;j<40;j++)
		{
			da=qqimage[k*2+1];
			da<<=8;
			da|=qqimage[k*2]; 
			SPI_LCD_RAM[i*40 + j] = da;					
			k++;  			
		}
	}
	LCD_Color_Fill(x,y,x+39,y+39,SPI_LCD_RAM);
}
#endif


