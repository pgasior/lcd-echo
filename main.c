#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <wiringPi.h>
#include <lcd.h>
#include <sr595.h>


/*
TODO:
    -dodac obsloge expanderow
    -pin startowy dla SR/expandera
    -config
    -customowe znaki
    -wlasny parser opcji
*/
int split_pins(char* value, int lcd_pins[8]);

int main(int argc, char **argv)
{
    int c;
    static int shift_register = 0;
    int clear_flag = 1;
    int width = 16;
    int height = 2;
    int pin_mode = 0;
    int x = 0;
    int y = 0;
    char *subopts;
    char *value;
    int option_index = 0;
    int sr_data = 0, sr_latch = 0, sr_clock = 0, sr_base = 100;
    int lcd_rs = 0, lcd_strb = 0;
    int lcd_pins[] = {0,0,0,0,0,0,0,0};
    
    
    
    
    static struct option long_options[] = 
    {
        {"shift-register", required_argument, &shift_register, 1},
        {"clear", required_argument, 0, 'c'},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"pins", required_argument, 0, 'p'},
        {"pos-x", required_argument, 0, 'x'},
        {"pos-y", required_argument, 0, 'y'},
        {"pin-mode", required_argument,0 ,'m'},
        {0,0,0,0}
    };
    
    char *sr_options[] = {"data", "latch", "clock", "base"};
    char *pins_numbers[] = {"rs", "strb", "pins"};
    while((c = getopt_long(argc, argv, "c:h:m:p:w:x:y:", long_options, &option_index))!= -1)
    {
        switch(c)
        {
            case 0:
                if(shift_register)
                {
                    subopts = optarg;
                    while(*subopts!='\0')
                        switch(getsubopt(&subopts, sr_options, &value))
                        {
                            case 0:
                                if(value == NULL)
                                    abort();
                                sr_data = atoi(value);
                                break;
                            case 1:
                                if(value == NULL)
                                    abort();
                                sr_latch = atoi(value);
                                break;
                            case 2:
                                if(value == NULL)
                                    abort();
                                sr_clock = atoi(value);
                                break;
                            case 3:
                                if(value == NULL)
                                    abort();
                                sr_base = atoi(value);
                                break;
                            default:
                                printf("Unknown suboption '%s'\n",value);
                                break;
                        }
                }
                break;
            case 'c':
                clear_flag = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
                break;
            case 'm':
                pin_mode = atoi(optarg);
                break;
            case 'p':
                subopts = optarg;
                while(*subopts!='\0')
                    switch(getsubopt(&subopts, pins_numbers, &value))
                    {
                        case 0:
                            if(value == NULL)
                                abort();
                            lcd_rs = atoi(value);
                            break;
                        case 1:
                            if(value == NULL)
                                abort();
                            lcd_strb = atoi(value);
                            break;
                        case 2:
                            if(value == NULL)
                                abort();
                            //printf("switch: %s\n",value);
                            split_pins(value,lcd_pins);
                            break;
                    }
                break;
            case 'w':
                width = atoi(optarg);
                break;
            case 'x':
                x = atoi(optarg);
            case 'y':
                y = atoi(optarg);
                break;
            default:
                abort();
        }
    }
    
    int argind = optind;
    
    switch(pin_mode)
    {
        case 0:
            wiringPiSetup();
            break;
        case 1:
            wiringPiSetupGpio();
            break;
        case 2:
            wiringPiSetupPhys();
            break;
        default:
            abort();
    }
    //printf("SR %d, base %d, data %d, clock %d, latch %d\n",shift_register,sr_base,sr_data,sr_clock,sr_latch);
    if(shift_register)
        sr595Setup(sr_base,8,sr_data,sr_clock,sr_latch);
        


    
    int lcd = lcdInit(height,width,4,lcd_rs,lcd_strb,lcd_pins[0],lcd_pins[1],lcd_pins[2],lcd_pins[3],lcd_pins[4],lcd_pins[5],lcd_pins[6],lcd_pins[7]);
    if(clear_flag)
        lcdClear(lcd);
    if(x!=0 || y!=0)
        lcdPosition(lcd,x,y);
        
    while(argind<argc)
    {
        lcdPrintf(lcd,"%s",argv[argind]);
        argind++;
        //printf("argind: %d, argc: %d\n",argind,argc);
        if(argind<argc-1)
            lcdPrintf(lcd," ");
    }
    return 0;
}

int split_pins(char* value, int lcd_pins[8])
{
    int pins = 0;
    int curr_pin = 0;
    //printf("%s\n",value);
    while(pins<=7)
    {
        //printf("val: %s\n", value);
        // if( !((*value>'0') && (*value<'9')) && (*value==' ') && (*value == '.'))
        // {
            // printf("Unrecognised symbol '%c'",*value);
            // abort();
        // }
        
        while((*value>='0') && (*value<='9'))
        {
            if(*value=='.')
            {
                value++;
                break;
            }
            while(*value==' ')
                value++;
            curr_pin = 10*curr_pin+(*value-'0');
            
            value++;
            
        }
        //printf("curr_pin = %d\t\tpins = %d\n",curr_pin,pins);
        lcd_pins[pins] = curr_pin;
        curr_pin = 0;
        value++;
        
        pins++;
        
    }
    return pins;
}
