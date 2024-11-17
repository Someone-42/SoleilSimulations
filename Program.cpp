#include <cmath>
#include <memory>
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
protected:
  // Time since epoch captured upon call of Start()
  uint64_t startTime;
  // Time in ms since simulation was started
  uint32_t timeElapsed;

public:
  PixelGameEngine* engine;
  Sprite* view = nullptr;
  Decal* viewDecal = nullptr;
  vi2d viewPos;

  Simulation(PixelGameEngine* engine)
  {
    this->engine = engine;
  }

  void UpdateView(Sprite* view, vu2d viewPos)
  {
    delete this->viewDecal;
    delete this->view;
    this->view = view;
    this->viewDecal = new Decal(view);
    this->viewPos = viewPos;
  }

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

class Component
{
public:
  Simulation* simulation;

  Pixel color;

  vi2d pos;
  vi2d size;

  Component(Simulation* simulation)
  {
    this->simulation = simulation;
  }

  Component(Simulation* simulation, vi2d pos, vi2d size)
  {
    this->simulation = simulation;
    this->pos = pos;
    this->size = size;
  }

  void Draw()
  {
    simulation->engine->DrawRect(simulation->viewPos + pos, size, color);
  }

  bool Collision(Component other)
  {
    // TODO:
    return false;
  }
};

class MagnetSimulation : public Simulation
{
public:
  Pixel color;

  Component magnet = Component(this);
  Component pipe = Component(this);

  MagnetSimulation(PixelGameEngine* engine, Pixel color) : Simulation(engine)
  {
    this->color = color;
  }

  ~MagnetSimulation()
  {
    delete this->viewDecal;
    delete this->view;
  }

  void Reset() override
  {
    // Resize components accordingly
    int x = view->Size().y / 10;
    magnet.size = vi2d(x, x);
    magnet.pos = vi2d(view->Size().x / 2, 2 * x) - (magnet.size / 2);

    pipe.size = vi2d(2 * x, view->Size().y * 0.8);
    pipe.pos = vi2d(view->Size().x / 2, view->Size().y / 2) - pipe.size / 2;
  }

  void Start() override
  {
    Simulation::Start();
  }

  void Update(float deltaTime) override
  {
    Simulation::Update(deltaTime);
  }

  void Render() override
  {
    engine->Clear(color);

    pipe.Draw();
    magnet.Draw();

    viewDecal->Update();
  }
};

class SimulationWindow : public olc::PixelGameEngine
{
private:
  vector<unique_ptr<Simulation>> simulations;

  Pixel clearColor = Pixel(0, 0, 0);

public:
  SimulationWindow()
  {
    cout << "ctor\n";
    sAppName = "Simulation";
    simulations.emplace_back(new MagnetSimulation(this, Pixel(255, 0, 0)));
    simulations.emplace_back(new MagnetSimulation(this, Pixel(0, 255, 0)));
    cout << "emplaced \n";
    // TODO: Create default views -> Update simulation view
    cout << "out ctor\n";
  }

  ~SimulationWindow()
  {
  }

public:
  bool OnUserCreate() override
  {
    simulations[0]->UpdateView(new Sprite(ScreenWidth() / 2, ScreenHeight()), vu2d(0, 0));
    simulations[1]->UpdateView(new Sprite(ScreenWidth() / 2, ScreenHeight()), vu2d(ScreenWidth() / 2, 0));
    for (auto& simulation : simulations)
    {
      simulation->Reset();
      simulation->Start();
    }
    cout << "out create\n";
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // ---------------------
    //      Update Cycle
    // ---------------------
    for (auto& simulation : simulations)
    {
      simulation->Update(fElapsedTime);
    }

    // ---------------------
    //      Render Cycle
    // ---------------------
    for (auto& simulation : simulations)
    {
      SetDrawTarget(simulation->view);
      simulation->Render();
      SetDrawTarget(nullptr);
      DrawDecal(simulation->viewPos, simulation->viewDecal);
    }

    // Going back to main screen for rendering

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
