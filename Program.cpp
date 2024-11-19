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
  // Sprite* view = nullptr;
  // Decal* viewDecal = nullptr;
  vi2d viewPos;
  vi2d viewSize;

  Simulation(PixelGameEngine* engine)
  {
    this->engine = engine;
  }

  void UpdateView(vf2d viewPos, vi2d viewSize)
  {
    this->viewPos = viewPos;
    this->viewSize = viewSize;
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

  vf2d pos;
  vf2d size;

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

  virtual void Draw()
  {
    simulation->engine->FillRectDecal(simulation->viewPos + pos, size, color);
  }

  bool Collision(Component other)
  {
    bool hitx = pos.x + size.x >= other.pos.x && other.pos.x + other.size.x >= pos.x;
    bool hity = pos.y + size.y >= other.pos.y && other.pos.y + other.size.y >= pos.y;
    return hitx && hity;
  }
};

class CopperPipe : public Component
{

public:
  CopperPipe(Simulation* simulation) : Component(simulation)
  {
  }

  Pixel innerColor;
  void Draw() override
  {
    vf2d sz = vf2d(size.x / 2, size.y);
    simulation->engine->GradientFillRectDecal(simulation->viewPos + pos, sz, color, color, innerColor, innerColor);
    simulation->engine->GradientFillRectDecal(simulation->viewPos + pos + vf2d(size.x / 2, 0), sz, innerColor,
                                              innerColor, color, color);
  }
};

class MagnetSimulation : public Simulation
{
public:
  Pixel color;

  Component pipePVC = Component(this);
  CopperPipe pipeCopper = CopperPipe(this);

  const float g = 9.81;
  float timeScale = 20;

  Component magnet = Component(this);
  float magnetSpeed = 0;
  vf2d magnetStartPos[2];
  char magnetCurrentPos = 0;

  MagnetSimulation(PixelGameEngine* engine, Pixel color) : Simulation(engine)
  {
    this->color = color;
    magnet.color = Pixel(165, 165, 165);
    pipeCopper.color = Pixel(184, 115, 51);
    pipeCopper.innerColor = Pixel(150, 50, 20);
    pipePVC.color = Pixel(128, 128, 160);
  }

  ~MagnetSimulation()
  {
  }

  void Reset() override
  {
    // Resize components accordingly
    int x = viewSize.y / 10;

    pipePVC.size = vi2d(1.5 * x, viewSize.y * 0.5);
    pipePVC.pos = vi2d(viewSize.x / 4, viewSize.y * 0.7) - pipePVC.size / 2;

    pipeCopper.size = vi2d(1.5 * x, viewSize.y * 0.5);
    pipeCopper.pos = vi2d(3 * viewSize.x / 4, viewSize.y * 0.7) - pipeCopper.size / 2;

    magnet.size = vi2d(1 * x, x / 3);
    magnet.pos = vi2d(pipePVC.pos.x + pipePVC.size.x / 2, viewSize.y * 0.05) - (magnet.size / 2);
    magnetStartPos[0] = magnet.pos;
    magnetStartPos[1] = magnet.pos + (vf2d(1, 0) * (-pipePVC.pos.x + pipeCopper.pos.x));
  }

  void Start() override
  {
    Simulation::Start();
  }

  void ChangeMagnetPipe()
  {
    magnetSpeed = 0;
    magnetCurrentPos = 1 - magnetCurrentPos;
    magnet.pos = magnetStartPos[magnetCurrentPos];
  }

  void Update(float deltaTime) override
  {
    Simulation::Update(deltaTime);

    deltaTime *= timeScale;

    float acc = g;

    if (magnet.Collision(pipeCopper))
    {
      // This is a lorentz force simulation
      // At some velocity, the induced magnetic field, creates opposing force equal to gravity
      // therefore the accelerations returns to 0
      //
      // And the magnet should move at constant speeds
      //
      // Therefore we multiply by some kind of simulation constant
      acc -= 0.8 * magnetSpeed;
    }

    magnetSpeed += deltaTime * acc;

    magnet.pos.y += magnetSpeed * deltaTime;

    if (magnet.pos.y >= viewSize.y)
      ChangeMagnetPipe();
  }

  void Render() override
  {
    engine->FillRectDecal(viewPos, viewSize, color);

    pipePVC.Draw();
    pipeCopper.Draw();
    magnet.Draw();

    // viewDecal->Update();
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
    simulations.emplace_back(new MagnetSimulation(this, Pixel(0, 0, 0)));
    simulations.emplace_back(new MagnetSimulation(this, Pixel(0, 0, 0)));
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
    simulations[0]->UpdateView(vi2d(0, 0), vi2d(ScreenWidth() / 2, ScreenHeight()));
    simulations[1]->UpdateView(vi2d(ScreenWidth() / 2, 0), vi2d(ScreenWidth() / 2, ScreenHeight()));
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
      // SetDrawTarget(simulation->view);
      simulation->Render();
      // DrawDecal(simulation->viewPos, simulation->viewDecal);
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
