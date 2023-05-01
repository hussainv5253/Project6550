#include <stdio.h>

#define MAX_WEIGHT 16

double w11[MAX_WEIGHT] = { 0.00049743426,0.019374547,0.26337504,-0.4819307,0.22693759,-0.20991307,0.3498497,-0.20577136,-0.19983867,1.2884545e-05,0.000008324,0.92378223,0.13489202,0.31475008,0.075073525,-0.52927685 };

double w12[MAX_WEIGHT] = { 0.09009434,0.2005909,0.04955896,-0.30565536,-0.10494534,0.82671323,0.10650399,-0.5047261,0.51268595,0.26337504,0.1230681,0.17693114,-0.16795921,0.0008922584,0.5525785,-0.39894873 };

double b1[MAX_WEIGHT] = { -0.31062353,-0.7632831,-0.8723926473,0.0,-0.06480652,-0.036529228,0.15216145,0.0,0.31815815,-0.09603859,-0.055650216,0.14971517,-0.21618177,-0.13414462,-0.13581157,0.0 };

double w2[MAX_WEIGHT] = { -0.012656047,-0.7813199,0.58553135,-0.12113628,-0.00403408,0.001297456,0.39760748,-0.052286047,0.3090558,-0.10213839,-0.42575318,0.4758378,0.09830533,-0.5111308,-0.48957595,-0.04498571 };

double b2 = 0.169741184;

int predict(double temp, double humidity)
{
	double result = 0;

	for (int i = 0; i < MAX_WEIGHT; i++)
	{
		double tmp = (temp*w11[i] + humidity*w12[i] + b1[i]);

		tmp = tmp < 0 ? 0 : tmp; // relu

		result += tmp*w2[i];
	}

	result += b2;

	return result < 0.5 ? 0 : 1; 
}

int main()
{
	// Prediction in pyton
	// abs(model.predict([[150,40]]).round())

	int temperature[3] = { 80, 210, 10 };
	int humidity[3] = { 10, 40, 30 };

	for (int i = 0; i < 3; i++)
	{
		printf("Temp: %d\tHumidity: %d\tHumidifier: %d\n", temperature[i], humidity[i], predict(temperature[i], humidity[i]));
	}

	return 0;
}
