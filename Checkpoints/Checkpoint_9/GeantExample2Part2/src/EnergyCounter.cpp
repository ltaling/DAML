#include "EnergyCounter.h"

#include "g4csv.hh"

EnergyCounter::EnergyCounter( const G4String& name, const G4int id )
  : G4VSensitiveDetector( name ) // Run the constructor of the parent class
{
  // Set which histogram ID / ntuple column to use
  m_ID = id;
}

EnergyCounter::~EnergyCounter()
{
}

// At the start of the event, zero the energy counter
void EnergyCounter::Initialize( G4HCofThisEvent* )
{
  m_totalEnergy = 0.0;
  m_detEnergy = 0.0;
}

// Analyse anything that hits the detector
G4bool EnergyCounter::ProcessHits( G4Step* step, G4TouchableHistory* )
{
  // Get the energy deposited by this hit
  G4double edep = step->GetTotalEnergyDeposit();
  
  // Get the track that this step belongs to
  G4Track * track = step->GetTrack();

  // Get the type of particle that made this track
  std::string particle = track->GetParticleDefinition()->GetParticleName();

  // Add to the total energy in this object
  m_totalEnergy += edep;
  
  // Add energy to detEnergy only if particle is an electron
  if(particle == "e-"){
	  m_detEnergy += edep;
  }

  return true;
}

// At the end of an event, store the energy collected in this detector
void EnergyCounter::EndOfEvent( G4HCofThisEvent* )
{
  // Get the analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  // Fill histogram (histogram 0, bin by layer ID)
  analysisManager->FillH1( 0, m_ID, m_totalEnergy );

  // Fill ntuple (it's always ntuple 0, column by layer ID)
  analysisManager->FillNtupleDColumn( 0, m_ID, m_totalEnergy );
  
  // Fill ntuple (for csv file number 2 (or 1 in computer language), column by layer ID)
  analysisManager->FillNtupleDColumn( 1, m_ID, m_detEnergy );
  
  
}
