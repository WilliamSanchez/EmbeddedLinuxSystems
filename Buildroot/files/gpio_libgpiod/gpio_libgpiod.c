#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	const char *chipname = "gpiochip0";
	struct gpiod_chip *chip;
	struct gpiod_line *led;
	struct gpiod_line *button;
	
	int i, ret, val;

	chip = gpiod_chip_open_by_name(chipname);
	if(!chip)
	{
		perror("Open chip failed\n");
		return 1;
	}

	led = gpiod_chip_get_line(chip,12);
	if(!led)
	{
		perror("Get line failed\n");
		return 1;
	}

	button = gpiod_chip_get_line(chip, 13);
	if(!button)
	{
		perror("get line failed\n");
		return 1;
	}

	ret = gpiod_line_request_output(led,"led_1",1);
	if(ret < 0)
	{
		perror("Request line as output failed\n");
		return 1;
	}

        ret = gpiod_line_request_input(button,"button_1");
        if(ret < 0)
        {
                perror("Request line as output failed\n");
                return 1;
        }

	i = 0;
	while(true)
	{
	        ret = gpiod_line_set_value(led,(i & 1) != 0);
        	if(ret < 0)
       		 {
                	perror("set line failed\n");
                	return 1;
        	}

	        val=gpiod_line_get_value(button);
        	if(val < 0)
        	{ 
                	perror("Read line input failed\n");
               	 	return 1;
        	}

		printf("Value is %d\n",val);
        	if (val == 0)
        	{ 
        		printf("EXIT \n");
                	break;
        	}

	   usleep(100000);
	  i++;
	}

	gpiod_line_release(led);
	gpiod_line_release(button);
	return 0;
}
