#include <stdint.h>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using namespace olc;
using namespace std;

uint64_t GetTimems()
{
  return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

class Simulation
{
private:
  // Time since epoch captured upon call of Start()
  uint64_t startTime;
  // Time in ms since simulation was started
  uint32_t timeElapsed;

public:
  PixelGameEngine* engine;
  Sprite* view;
  vu2d viewPos;

  Simulation(PixelGameEngine* engine)
  {
    this->engine = engine;
  }

  virtual void UpdateView(Sprite* view, vu2d viewPos);

  virtual void Reset();

  // Called to create and start the simulation
  virtual void Start()
  {
    startTime = GetTimems();
  }

  virtual void Update(float deltaTime)
  {
    timeElapsed = GetTimems() - timeElapsed;
  }

  virtual void Render();
};

class SimulationWindow : public olc::PixelGameEngine
{
private:
  vector<Simulation*> simulations;

  Pixel clearColor = Pixel(0, 0, 0);

public:
  SimulationWindow()
  {
    sAppName = "Simulation";
    simulations = {};
    // TODO: Create default views -> Update simulation view
  }

  ~SimulationWindow()
  {
    for (Simulation* simulation : simulations)
    {
      delete simulation;
    }
  }

public:
  bool OnUserCreate() override
  {
    for (Simulation* simulation : simulations)
    {
      simulation->Reset();
      simulation->Start();
    }
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // ---------------------
    //      Update Cycle
    // ---------------------
    for (Simulation* simulation : simulations)
    {
      simulation->Update(fElapsedTime);
    }

    // ---------------------
    //      Render Cycle
    // ---------------------
    Clear(clearColor);
    for (Simulation* simulation : simulations)
    {
      simulation->Render();
      DrawSprite(simulation->viewPos.x, simulation->viewPos.y, simulation->view);
    }
    // TODO: Draw delimiter ? UI ?
    return true;
  }
};

int main()
{
  SimulationWindow window;
  if (window.Construct(1200, 800, 1, 1))
    window.Start();

  return 0;
}
