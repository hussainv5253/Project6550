// SOURCE: https://www.kernel.org/doc/Documentation/i2c/dev-interface */
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <i2c/smbus.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <csignal>

volatile sig_atomic_t flag = 0;

//ML C++ code
#define MAX_WEIGHT 16
double w11[MAX_WEIGHT] = { 0.00049743426,0.019374547,0.26337504,-0.4819307,0.22693759,-0.20991307,0.3498497,-0.20577136,-0.19983867,1.2884545e-05,0.000008324,0.92378223,0.13489202,0.31475008,0.075073525,-0.52927685 };
double w12[MAX_WEIGHT] = { 0.09009434,0.2005909,0.04955896,-0.30565536,-0.10494534,0.82671323,0.10650399,-0.5047261,0.51268595,0.26337504,0.1230681,0.17693114,-0.16795921,0.0008922584,0.5525785,-0.39894873 };
double b1[MAX_WEIGHT] = { -0.31062353,-0.7632831,-0.8723926473,0.0,-0.06480652,-0.036529228,0.15216145,0.0,0.31815815,-0.09603859,-0.055650216,0.14971517,-0.21618177,-0.13414462,-0.13581157,0.0 };
double w2[MAX_WEIGHT] = { -0.012656047,-0.7813199,0.58553135,-0.12113628,-0.00403408,0.001297456,0.39760748,-0.052286047,0.3090558,-0.10213839,-0.42575318,0.4758378,0.09830533,-0.5111308,-0.48957595,-0.04498571 };
double b2 = 0.169741184;

// int file;
int adapter_nr = 1;
char filename[20];
int rate;
bool on;
int temp;
int hum;
int light;
int gp17, gp22, i2c;

using namespace std;

pthread_mutex_t sensorLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lightLock = PTHREAD_MUTEX_INITIALIZER;


void* readI2C(void* args){

    i2c = open("/dev/i2c-1", O_RDWR);
    if (i2c < 0) {
        exit(1);
    }
    int addr = 0x9;

    if (ioctl(i2c, I2C_SLAVE, addr) < 0) {
        exit(1);
    }
    usleep(4000000);
    unsigned char buffer[0x10];
    buffer[0] = 0b00000001;
    buffer[1] = 0b00000100;
    buffer[2] = 0b00000011;
    write(i2c, buffer, 3);
    char* token;
    for (;;)
    {
        //Read I2C
        char buff[17];
        if (read(i2c, buff, sizeof(buff)) != sizeof(buff)) {
            perror("Failed to read from I2C bus");
            close(i2c);
            exit(1);
        }
        const char* myChar = buff;
        // printf("Message is %s\n", myChar);

        token = strtok((char*)myChar, " ");
        int count = 0;
        pthread_mutex_lock(&sensorLock);
        while (token != NULL) { 
            if (count == 1)
                hum =  atoi(token);
            else if (count == 3)
                temp = atoi(token);
            else if (count == 5)
                light = atoi(token);

            count ++;
            token = strtok(NULL, " ");
        }
        // cout << "Humidity: " << hum << " Temperature: " << temp << " Brightness: " << light << endl;
        pthread_mutex_unlock(&sensorLock);
        usleep(rate * 1000000);
    }
}

void* controlLight(void* arg)
{
    gp17 = open("/sys/class/gpio/gpio17/value", O_WRONLY);
    bool prevMode = false;
    int tmp;
    on = true;
    usleep(100000);

    for(;;)
    {
        //lock status of on
        pthread_mutex_lock(&lightLock);
        bool tempMode = on;
        pthread_mutex_unlock(&lightLock);
        
        if (!tempMode) //On means currently Off
        {
            write(gp17, "0", 1); //Off
        } else {
            pthread_mutex_lock(&sensorLock);
            tmp = light;
            pthread_mutex_unlock(&sensorLock);
            write(gp17, "1", 1);//ON
            usleep(10000 - tmp*100); 
            write(gp17, "0", 1); //Off
            usleep(10000 + tmp*400);
        }
    }
    close(gp17);
}

int predict(double temperature, double humidity)
{
	double result = 0;

	for (int i = 0; i < MAX_WEIGHT; i++)
	{
		double tmp = (temperature*w11[i] + humidity*w12[i] + b1[i]);

		tmp = tmp < 0 ? 0 : tmp; // relu

		result += tmp*w2[i];
	}

	result += b2;

	return result < 0.5 ? 0 : 1; 
}

void* controlHumidifier(void* arg)
{
    gp22 = open("/sys/class/gpio/gpio22/value", O_WRONLY);
    bool prevMode = false;
    int res;

    for(;;)
    {
        //lock status of on
        pthread_mutex_lock(&sensorLock);
        res = predict(temp, hum);
        pthread_mutex_unlock(&sensorLock);
        //Unlock
        if (res)
            write(gp22, "1", 1);//ON
        else
            write(gp22, "0", 1); //Off
        usleep(rate * 10000000);
        write(gp22, "0", 1); //Off
        usleep(60000000);

    }
    close(gp22);
}

void* toggleLED(void* arg)
{
    char t;
    while (1)
    {
        //If we send shutdown signal
        cout << "Toggle LED (y)" << endl;
        scanf("%c", &t);
        if (t == 'y')
        {
            pthread_mutex_lock(&lightLock);
            on = !on;
            pthread_mutex_unlock(&lightLock);
        }
    }
}

void handle_ctrlc(int sig) {
    cout << "CTRL+C signal received" << endl;
    write(gp17, "0", 1); //Off
    write(gp22, "0", 1); //Off
    close(gp17);
    close(gp22);
    exit(1);
}

int main()
{
    signal(SIGINT, handle_ctrlc);
    printf("Enter the sensing and actuation rate in seconds: ");
    scanf("%d", &rate);

    int gpio = open("/sys/class/gpio/export", O_WRONLY);
    write(gpio, "17", 2);
    write(gpio, "22", 2);
    close(gpio);

    //PWM Lights
    int gp17 = open("/sys/class/gpio/gpio17/direction", O_WRONLY);
    write(gp17, "out", 3);
    close(gp17);

    //Humidifier LED (On/Off)
    int gp22 = open("/sys/class/gpio/gpio22/direction", O_WRONLY);
    write(gp22, "out", 3);
    close(gp22);

    pthread_t t1, t2, t3, t4;
    void* exitStatus;
    int x1 = 1, x2 = 2, x3 = 3, x4;
    pthread_create(&t1, NULL, readI2C, &x1);
    pthread_create(&t1, NULL, controlLight, &x2);
    pthread_create(&t1, NULL, controlHumidifier, &x3);
    pthread_create(&t1, NULL, toggleLED, &x4);

    while(!flag)
    {

    }

    pthread_join(t1, &exitStatus);
    pthread_join(t2, &exitStatus);
    pthread_join(t3, &exitStatus);
    pthread_join(t4, &exitStatus);

    return 0;
}

//To run, g++ -o i2c i2c.cpp
//./i2c