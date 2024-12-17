#include <zephyr/kernel.h>
#include <math.h>
#include "confusion.h"
#include "adc.h"
#include "keskipisteet4.h"


/* 
  K-means algorithm should provide 6 center points with
  3 values x,y,z. Let's test measurement system with known
  center points. I.e. x,y,z are supposed to have only values
  1 = down and 2 = up
  
  CP matrix is thus the 6 center points got from K-means algoritm
  teaching process. This should actually come from include file like
  #include "KmeansCenterPoints.h"
  
  And measurements matrix is just fake matrix for testing purpose
  actual measurements are taken from ADC when accelerator is connected.
*/ 

/*int CP[6][3]={
	                     {1,0,0},
						 {2,0,0},
						 {0,1,0},
						 {0,2,0},
						 {0,0,1},
						 {0,0,2}
};*/

int measurements[6][3]={
	                     {1,0,0},
						 {2,0,0},
						 {0,1,0},
						 {0,2,0},
						 {0,0,1},
						 {0,0,2}
};

int CM[6][6]= {0};



void printConfusionMatrix(void)
{
	printk("Confusion matrix = \n");
	printk("   cp1 cp2 cp3 cp4 cp5 cp6\n");
	for(int i = 0;i<6;i++)
	{
		printk("cp%d %d   %d   %d   %d   %d   %d\n",i+1,CM[i][0],CM[i][1],CM[i][2],CM[i][3],CM[i][4],CM[i][5]);
	}

}

void makeHundredFakeClassifications(void)
{
   /*******************************************
   Jos ja toivottavasti kun teet toteutuksen paloissa eli varmistat ensin,
   että etäisyyden laskenta 6 keskipisteeseen toimii ja osaat valita 6 etäisyydestä
   voittajaksi sen lyhyimmän etäisyyden, niin silloin voit käyttää tätä aliohjelmaa
   varmistaaksesi, että etäisuuden laskenta ja luokittelu toimii varmasti tunnetulla
   itse keksimälläsi sensoridatalla ja itse keksimilläsi keskipisteillä.
   *******************************************/


    printk("Starting 100 fake classifications...\n");

    for (int i = 0; i < 100; i++)
    {
        // Step 1: Select a fake measurement from CP (cycle through center points)
        int true_class = i % 6;  // Cycle through 0 to 5
        int x = CP[true_class][0];
        int y = CP[true_class][1];
        int z = CP[true_class][2];

        // Step 2: Classify the measurement
        int predicted = calculateDistanceToAllCentrePointsAndSelectWinner(x, y, z);

        // Step 3: Update the confusion matrix
        if (true_class >= 0 && true_class < 6 && predicted >= 0 && predicted < 6) // Safety check
        {
            CM[true_class][predicted]++;
        }
        else
        {
            printk("Error: Invalid indices - True class = %d, Predicted = %d\n", true_class, predicted);
        }
    }

    printk("Finished 100 fake classifications.\n");
    printConfusionMatrix();
}

void takeHundredRealMeasurementsAndUpdateConfusionMatrix(int direction)
{
    int measurement_count = 0;  // Counter for measurements

    printk("Starting 100 real measurements...\n");

    while (measurement_count < 100)
    {
        // Simulate reading from ADC (you can replace this with actual ADC reading function)
        struct Measurement m = readADCValue();  // Replace this with your actual ADC reading function
        printk("Measurement %d: x = %d, y = %d, z = %d\n", measurement_count + 1, m.x, m.y, m.z);

        // Determine the winner (closest center point)
        int predicted_class = calculateDistanceToAllCentrePointsAndSelectWinner(m.x, m.y, m.z);

        // Update the confusion matrix
        CM[direction][predicted_class]++;

        // Print the confusion matrix after each update
        printConfusionMatrix();

        // Increment measurement count
        measurement_count++;

        k_sleep(K_MSEC(50));  // Small delay between measurements (optional)
    }

    printk("Finished 100 real measurements.\n");
    printConfusionMatrix();
}

   //printk("Make your own implementation for this function if you need this\n");


void makeOneClassificationAndUpdateConfusionMatrix(int direction)
{
    if (direction < 0 || direction >= 6)
    {
        printk("Invalid direction: %d\n", direction);
        return;
    }

    // Simulated measurement from the measurements matrix
    int x = measurements[direction][0];
    int y = measurements[direction][1];
    int z = measurements[direction][2];

    // Determine the winner (closest center point)
    int predicted_class = calculateDistanceToAllCentrePointsAndSelectWinner(x, y, z);

    // Update the confusion matrix
    CM[direction][predicted_class]++;

    printk("One classification done: Actual = %d, Predicted = %d\n", direction, predicted_class);
}


/*int calculateDistanceToAllCentrePointsAndSelectWinner(int x,int y,int z)
{
   ***************************************
   Tämän aliohjelma ottaa yhden kiihtyvyysanturin mittauksen x,y,z,
   laskee etäisyyden kaikkiin 6 K-means keskipisteisiin ja valitsee
   sen keskipisteen, jonka etäisyys mittaustulokseen on lyhyin.
   ***************************************/


int calculateDistanceToAllCentrePointsAndSelectWinner(int x, int y, int z)
{
    double distances[6];  // Store distances to each of the 6 center points
    int winner = 0;       // Index of the closest center point

    // Calculate Euclidean distances to all center points
    for (int i = 0; i < 6; i++)
    {
        int dx = x - CP[i][0];
        int dy = y - CP[i][1];
        int dz = z - CP[i][2];
        distances[i] = sqrt(dx * dx + dy * dy + dz * dz);
    }

    // Find the index of the smallest distance
    double min_distance = distances[0];
    for (int i = 1; i < 6; i++)
    {
        if (distances[i] < min_distance)
        {
            min_distance = distances[i];
            winner = i;
        }
    }

    printk("Measurement (%d, %d, %d) classified as center point %d (distance = %.2f)\n",
           x, y, z, winner + 1, min_distance);

    return winner;
}

   
   /*printk("Make your own implementation for this function if you need this\n");
   return 0;*/


void resetConfusionMatrix(void)
{
	for(int i=0;i<6;i++)
	{ 
		for(int j = 0;j<6;j++)
		{
			CM[i][j]=0;
		}
	}
}

