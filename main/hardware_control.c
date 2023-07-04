#include "main.h"
#include "hardware_control.h"


void hardware_control_fun(void *parameters){

    
init_blocator_gpios();

vTaskDelay(300/portTICK_PERIOD_MS);

while(1){
/*
firstly, get the command to be exefcuted from command queue
*/
char current_command=0;

if(xQueueReceive(command_queue, (void *)&current_command, 0)==pdTRUE){
    //a valid comand was received from queue
    ESP_LOGI("HW","command received");
}


uint8_t rec_command=COMMAND_UNDEFINED;

switch (current_command)
{
case (char)'u':
    rec_command=COMMAND_UP;
    break;

case (char)'d':
    rec_command=COMMAND_DOWN;
    break;

default:
    rec_command=COMMAND_UNDEFINED;
    break;
}

/*
execution of command
*/

if(rec_command==COMMAND_UP){

    while(execute_command_up()!=0){
        ESP_LOGI("*","");

        vTaskDelay(150/portTICK_PERIOD_MS);


        /*
        add custom code to be executed on movement
        */
       on_movement_common();

        if(is_motor_blocked())break;


       /**/

    };

    if(execute_command_up()==0){
        ESP_LOGI("\n execution UP done","");
    }    
    else{
        ESP_LOGE("\n Motor Current","HIGH STOP");
        execute_command_undef();
    }

}
else if(rec_command==COMMAND_DOWN){

     while(execute_command_down()!=0){
        ESP_LOGI("*","");

        vTaskDelay(150/portTICK_PERIOD_MS);


        /*
        add custom code to be executed on movement
        */
       on_movement_common();

        if(is_motor_blocked())break;

       /**/
       
    };

    if(execute_command_down()==0){
        ESP_LOGI("\n execution DOWN done \n","");
    }
    else{
        ESP_LOGE("\n Motor Current","HIGH STOP");
        execute_command_undef();
    }
    
}
else{
    ESP_LOGE("\n command undefined","");
    execute_command_undef();
}

vTaskDelay(100/portTICK_PERIOD_MS);

}
 
}

void on_movement_common(){
    /*
    report current on motor
    */
    uint16_t motor_milliamps = get_motor_milliamps();
    ESP_LOGI("Motor_current"," %d",motor_milliamps);

}

/*
executes
    blocator up
returns 
    0 on execution success
    1 on exec start -- pending
    2 on error
*/
uint8_t execute_command_up(){

    /*
    verify if blocator is already in up position
    */
   if(gpio_get_level(SW_UP_GPIO)==0){
    /*
    make sure relays are turned off
    */
    gpio_set_level(RELAY_UP_GPIO, 0);
    gpio_set_level(RELAY_DOWN_GPIO, 0);

    return 0;  //return 0 on success
   }

   /*
   verify if blocator is in down position
   */
  if(gpio_get_level(SW_DOWN_GPIO)==0){
    /*
    write on command on relay_up
    */
   gpio_set_level(RELAY_UP_GPIO, 1);

   /*
   return 1 -- execution pending
   */
  return 1;
  }

  /*
  verify if blocator is in middle position
  */
 if((gpio_get_level(SW_UP_GPIO)==1)&&(gpio_get_level(SW_DOWN_GPIO)==1)){
    if(gpio_get_level(RELAY_UP_GPIO)==1){
        /*
        blocator is in execution pending
        */
       return 1;
    }
    else{
        gpio_set_level(RELAY_UP_GPIO, 1);  //turning on the up relay
        return 1; //execution is pending
    }
 }

 /*
 by default return error
 */
return 2;  

}


/*
executes
    blocator down
returns 
    0 on execution success
    1 on exec start -- pending
    2 on error
*/
uint8_t execute_command_down(){

    /*
    verify if blocator is already in down position
    */
   if(gpio_get_level(SW_DOWN_GPIO)==0){
    /*
    make sure relays are turned off
    */
    gpio_set_level(RELAY_UP_GPIO, 0);
    gpio_set_level(RELAY_DOWN_GPIO, 0);

    return 0;  //return 0 on success
   }

   /*
   verify if blocator is in up position
   */
  if(gpio_get_level(SW_UP_GPIO)==0){
    /*
    write on command on relay_down
    */
   gpio_set_level(RELAY_DOWN_GPIO, 1);

   /*
   return 1 -- execution pending
   */
  return 1;
  }

  /*
  verify if blocator is in middle position
  */
 if((gpio_get_level(SW_UP_GPIO)==1)&&(gpio_get_level(SW_DOWN_GPIO)==1)){
    if(gpio_get_level(RELAY_DOWN_GPIO)==1){
        /*
        blocator is in execution pending
        */
       return 1;
    }
    else{
        gpio_set_level(RELAY_DOWN_GPIO, 1);  //turning on the down relay
        return 1; //execution is pending
    }
 }

 /*
 by default return error
 */
return 2;  

}

void execute_command_undef(){
    /*
    make sure both relays are off
    */
   gpio_set_level(RELAY_DOWN_GPIO,0);
   gpio_set_level(RELAY_UP_GPIO,0);

   return;
}



void init_blocator_gpios(){

gpio_set_direction(RELAY_DOWN_GPIO, GPIO_MODE_OUTPUT);
gpio_set_direction(RELAY_UP_GPIO, GPIO_MODE_OUTPUT);
gpio_set_direction(SW_DOWN_GPIO, GPIO_MODE_INPUT);
gpio_set_direction(SW_UP_GPIO, GPIO_MODE_INPUT);

/*
config to use adc1
*/
adc1_config_width(ADC_WIDTH_BIT_12);
adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);

/*
adc1 reading function
*/
// int val = adc1_get_raw(ADC1_CHANNEL_0);

vTaskDelay(100/portTICK_PERIOD_MS);

}

int get_motor_milliamps(){
    int val = adc1_get_raw(ADC1_CHANNEL_0);
    int return_val=(int)((float)val * (float)2.68);   //from testing, it gives a calibration factor
    return return_val; //current in milliamps
}

_Bool is_motor_blocked() {

#define MOTOR_CURRENT_MAX 2500

    int motor_current=get_motor_milliamps();
    
    if(motor_current > MOTOR_CURRENT_MAX)return true;

    return false;
}