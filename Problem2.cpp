#include <algorithm>
#include <climits>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <set>
#include <thread>

#define NUM_THREADS 8
#define NUM_MIN 60
#define NUM_HOURS 48

std::mutex mutex;

int generateRandomTemp(int minTemp, int maxTemp)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(minTemp, maxTemp);
  return dist(gen);
}

void printHighesttemps(std::vector<int> &sensors)
{
  std::set<int> temps{};

  for (auto it = sensors.rbegin(); it != sensors.rend(); it++)
  {
    if (temps.find(*it) == temps.end())
    {
      temps.insert(*it);
    }

    if (temps.size() == 5)
    {
      break;
    }
  }

  std::cout << "Highest temps: ";

  for (int temp : temps)
  {
    std::cout << temp << "F ";
  }

  std::cout << std::endl;
}

void printLowesttemps(std::vector<int> &sensors)
{
  std::set<int> temps{};

  for (auto it = sensors.begin(); it != sensors.end(); it++)
  {
    if (temps.find(*it) == temps.end())
    {
      temps.insert(*it);
    }

    if (temps.size() == 5)
    {
      break;
    }
  }

  std::cout << "Lowest temps: ";

  for (int temp : temps)
  {
    std::cout << temp << "F ";
  }

  std::cout << std::endl;
}

void generateHourlyReport(int hour, std::vector<int> &sensors)
{
  int endInterval = 10;
  int startInterval = 0;
  int maxDifference = 0;

  std::cout << "***Hour " << hour + 1 << " report***" << std::endl;
  std::cout << "-------------------------------" << std::endl;

  for (int threadIdx = 0; threadIdx < NUM_THREADS; threadIdx++)
  {
    int offset = threadIdx * NUM_MIN;

    for (int i = offset; i < NUM_MIN - endInterval + 1; i++)
    {
      int maxTemp = *std::max_element(sensors.begin() + i, sensors.begin() + i + endInterval);
      int minTemp = *std::min_element(sensors.begin() + i, sensors.begin() + i + endInterval);
      int tempDifference = maxTemp - minTemp;

      if (tempDifference > maxDifference)
      {
        maxDifference = tempDifference;
        startInterval = i;
      }
    }
  }

  std::sort(sensors.begin(), sensors.end());

  printHighesttemps(sensors);
  printLowesttemps(sensors);

  std::cout << "the largest temp difference was: " << maxDifference << "F"
            << " During minute " << startInterval << " through minute "
            << (startInterval + 10) << std::endl;

  std::cout << std::endl
            << std::endl;
}

void tempReadings(int threadId, std::vector<int> &sensors,
                  std::vector<bool> &flag)
{
  for (int hour = 0; hour < NUM_HOURS; hour++)
  {
    for (int minute = 0; minute < NUM_MIN; minute++)
    {
      flag[threadId] = false;
      sensors[minute + (threadId * NUM_MIN)] = generateRandomTemp(-100, 70);
      flag[threadId] = true;
    }

    if (threadId == 0)
    {
      mutex.lock();
      generateHourlyReport(hour, sensors);
      mutex.unlock();
    }
  }
}

int main()
{
  std::vector<int> sensors(NUM_THREADS * NUM_MIN);
  std::vector<bool> flag(NUM_THREADS);
  std::thread threads[NUM_THREADS] = {};

  for (int i = 0; i < NUM_THREADS; i++)
  {
    threads[i] =
        std::thread(tempReadings, i, std::ref(sensors), std::ref(flag));
  }

  auto start_time = std::chrono::high_resolution_clock::now();

  for (std::thread &thread : threads)
  {
    thread.join();
  }

  auto end_time = std::chrono::high_resolution_clock::now();

  auto execution_time = end_time - start_time;
  auto seconds =
      std::chrono::duration_cast<std::chrono::seconds>(execution_time);
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
      execution_time - seconds);

  std::cout << "It took " << seconds.count() << "." << milliseconds.count()
            << " seconds for the program to execute" << std::endl;

  return 0;
}