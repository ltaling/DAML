#include "DetectorConstruction.h"
#include "EnergyCounter.h"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4AutoDelete.hh"
#include "G4GeometryManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

// Set number of detector layers
G4int const nLayers = 5; // This means 5 layers as we count from 0

G4ThreadLocal
G4GlobalMagFieldMessenger* DetectorConstruction::m_magneticFieldMessenger = 0;

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction()
{
}

DetectorConstruction::~DetectorConstruction()
{
}

// Here we define the actual experiment that we want to perform
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Materials
  // http://geant4-userdoc.web.cern.ch/geant4-userdoc/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html
  G4NistManager* nistManager = G4NistManager::Instance();
  G4Material* vacuum = nistManager->FindOrBuildMaterial( "G4_Galactic" );
  G4Material* lead = nistManager->FindOrBuildMaterial( "G4_Pb" );
  G4Material* liquidArgon = nistManager->FindOrBuildMaterial( "G4_lAr" );
  G4Material* steel = nistManager->FindOrBuildMaterial( "G4_STAINLESS-STEEL" ); // Steel is a relatively light but strong material
  G4Material* tungsten = nistManager->FindOrBuildMaterial( "G4_W" ); // Tungsten emits many photons
  G4Material* graphite = nistManager->FindOrBuildMaterial( "G4_GRAPHITE" ); //Graphite emits many hadrons
  G4Material* beryllium = nistManager->FindOrBuildMaterial( "G4_Be" ); //Beryllium is often used as a target

  // Sizes of the principal geometrical components
  G4double length = 25.0*cm; // Half-length of the detector
  // Liquid Argon cannot float in mid-air so it must be within a volume
  G4double caseInner = 50.0*cm;
  G4double caseOuter = 55.0*cm;
  //  Liquid argon must remain inside this volume
  G4double homogeneousRadius = 50.0*cm;
  // Absorbers for hadronic sampling calorimeter
  G4double absorberRadius = 50.0*cm;
  G4double absorberThickness = 2.5*cm;
  // Detectors for hadronic sampling calorimeter
  G4double detectorRadius= 50.0*cm;
  G4double detectorThickness = 7.5*cm;
  // Target structure
  G4double targetRadius = 55.0*cm; // Keep it in line with the detector and shell
  G4double targetLength = 2.5*cm; // Make it 1cm long
  // World size
  G4double worldLength = 250.0*cm;

  // Definitions of Solids, Logical Volumes, Physical Volumes
  G4double zPosition_homogeneous = 30.0*cm;
  G4double zPosition_sampling = 90.0*cm;
  G4double zPosition_target = 0.0*cm;

  // WORLD: Solid (cube)
  G4GeometryManager::GetInstance()->SetWorldMaximumExtent( worldLength );
  G4Box* worldS = new G4Box(
                 "World",         // its name
                 worldLength,
                 worldLength,
                 worldLength );   // its size (in half-lengths)

  // WORLD: Logical volume (how to treat it)
  G4LogicalVolume* worldLV = new G4LogicalVolume(
                 worldS,          // its solid
                 vacuum,          // its material
                 "World" );       // its name

  // WORLD: Physical volume (where is it)
  // Must place the World Physical volume unrotated at (0,0,0).
  G4VPhysicalVolume* worldPV = new G4PVPlacement(
                 0,               // no rotation
                 G4ThreeVector(0.0, 0.0, 0.0), // in the centre
                 worldLV,         // its logical volume
                 "World",         // its name
                 0,               // its mother volume
                 false,           // no boolean operations
                 0,               // copy number
                 true );          // checking overlaps


  // Make the Target
  G4ThreeVector targetPosition( 0, 0, zPosition_target );

  G4Tubs* targetS = new G4Tubs(
                 "target",      // its name
                 0.0,               // inner radius
                 targetRadius,    // outer radius
                 targetLength, // how much material in the beam path (half length)
                 0.0*deg,           // starting angle
                 360.0*deg );       // ending angle (i.e. it's a full circle)

  // TARGET: Logical volume (how to treat it)
  G4LogicalVolume* targetLV = new G4LogicalVolume(
                 targetS,         // its solid
                 beryllium,             // its material
                 "target",      // its name
                 0, 0, 0 );         // Modifiers we don't use

  // TARGET: Physical volume (where is it)
  G4VPhysicalVolume* targetPV = new G4PVPlacement(
                 0,                 // no rotation
                 targetPosition,  // where is it
                 targetLV,        // its logical volume
                 "target",      // its name
                 worldLV,           // its mother volume
                 false,             // no boolean operations
                 0,                 // copy number
                 true );            // checking overlaps


  // Make the outer shell of the homogeneous calorimeter
  G4ThreeVector homogeneousPosition( 0, 0, zPosition_homogeneous );

  G4Tubs* homogeneousShellS = new G4Tubs(
                 "shell",      // its name
                 caseInner,               // inner radius
                 caseOuter,    // outer radius
                 length, // how much material in the beam path (half length)
                 0.0*deg,           // starting angle
                 360.0*deg );       // ending angle (i.e. it's a full circle)

   //DETECTOR: Logical volume (how to treat it)
  G4LogicalVolume* homogeneousShellLV = new G4LogicalVolume(
                 homogeneousShellS,         // its solid
                 steel,             // its material
                 "shell",      // its name
                 0, 0, 0 );         // Modifiers we don't use

  // DETECTOR: Physical volume (where is it)
  G4VPhysicalVolume* homogeneousShellPV = new G4PVPlacement(
                 0,                 // no rotation
                 homogeneousPosition,  // where is it
                 homogeneousShellLV,        // its logical volume
                 "shell",      // its name
                 worldLV,           // its mother volume
                 false,             // no boolean operations
                 0,                 // copy number
                 true );            // checking overlaps




   // Make the inside of the homogeneous calorimeter
   G4Tubs* homogeneousS = new G4Tubs(
                  "homogeneous",      // its name
                  0.0,               // inner radius
                  homogeneousRadius,    // outer radius
                  length, // how much material in the beam path (half length)
                  0.0*deg,           // starting angle
                  360.0*deg );       // ending angle (i.e. it's a full circle)

   // DETECTOR: Logical volume (how to treat it)
   G4LogicalVolume* homogeneousLV = new G4LogicalVolume(
                  homogeneousS,         // its solid
                  liquidArgon,             // its material
                  "homogeneous",      // its name
                  0, 0, 0 );         // Modifiers we don't use

   // DETECTOR: Physical volume (where is it)
   G4VPhysicalVolume* homogeneousPV = new G4PVPlacement(
                  0,                 // no rotation
                  homogeneousPosition,  // where is it
                  homogeneousLV,        // its logical volume
                  "homogeneous",      // its name
                  worldLV,           // its mother volume
                  false,             // no boolean operations
                  0,                 // copy number
                  true );            // checking overlaps




  // DETECTOR: Warn if there's an overlap
  if ( homogeneousShellPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;
  if ( homogeneousPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;
  if ( targetPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;

  // Make multiple layers for the sampling calorimeter
  // Define positions of the detector (centre)
  for ( unsigned int layerIndex = 1; layerIndex <= nLayers; ++layerIndex )
  {
    // ABSORBER: Layer properties
    std::string absorberName = "Absorber" + std::to_string( layerIndex );
    G4ThreeVector absorberPosition( 0, 0, zPosition_sampling );

    // ABSORBER: Solid (tube)
    G4Tubs* absorberS = new G4Tubs(
                   absorberName,      // its name
                   0.0,               // inner radius 0, so it's a solid cylinder (not a hollow tube)
                   absorberRadius,    // outer radius
                   absorberThickness, // how much material in the beam path (half length)
                   0.0*deg,           // starting angle
                   360.0*deg );       // ending angle (i.e. it's a full circle)

    // ABSORBER: Logical volume (how to treat it)
    G4LogicalVolume* absorberLV = new G4LogicalVolume(
                   absorberS,         // its solid
                   lead,              // its material
                   absorberName,      // its name
                   0, 0, 0 );         // Modifiers we don't use

    // ABSORBER: Physical volume (where is it)
    G4VPhysicalVolume* absorberPV = new G4PVPlacement(
                   0,                 // no rotation
                   absorberPosition,  // where is it
                   absorberLV,        // its logical volume
                   absorberName,      // its name
                   worldLV,           // its mother volume
                   false,             // no boolean operations
                   0,                 // copy number
                   true );            // checking overlaps

    zPosition_sampling += (absorberThickness+detectorThickness); //Shift the distance by a segment length

    // ABSORBER: Quit if there's an overlap
    if ( absorberPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;

    // DETECTOR: Layer properties
    std::string detectorName = "Detector" + std::to_string( layerIndex );

    G4ThreeVector detectorPosition( 0, 0, zPosition_sampling );

    // DETECTOR: Solid (tube)
    G4Tubs* detectorS = new G4Tubs(
                   detectorName,      // its name
                   0.0,               // inner radius 0, so it's a solid cylinder (not a hollow tube)
                   detectorRadius,    // outer radius
                   detectorThickness, // how much material in the beam path (half length)
                   0.0*deg,           // starting angle
                   360.0*deg );       // ending angle (i.e. it's a full circle)

    // DETECTOR: Logical volume (how to treat it)
    G4LogicalVolume* detectorLV = new G4LogicalVolume(
                   detectorS,         // its solid
                   liquidArgon,       // its material
                   detectorName,      // its name
                   0, 0, 0 );         // Modifiers we don't use

    // DETECTOR: Physical volume (where is it)
    G4VPhysicalVolume* detectorPV = new G4PVPlacement(
                   0,                 // no rotation
                   detectorPosition,  // where is it
                   detectorLV,        // its logical volume
                   detectorName,      // its name
                   worldLV,           // its mother volume
                   false,             // no boolean operations
                   0,                 // copy number
                   true );            // checking overlaps

    // DETECTOR: Warn if there's an overlap
    if ( detectorPV->CheckOverlaps() ) std::cerr << "WARNING: your simulated objects overlap" << std::endl;

    zPosition_sampling += (absorberThickness + detectorThickness); //Shift the distance by a segment length
  }

  // Always return the physical world
  return worldPV;
}

// Set up the magnetic field
void DetectorConstruction::ConstructSDandField()
{
  // Create global magnetic field messenger.
  // Uniform magnetic field is then created automatically if
  // the field value is not zero.
  G4ThreeVector fieldValue = G4ThreeVector();
  m_magneticFieldMessenger = new G4GlobalMagFieldMessenger( fieldValue );

  // Register the field messenger for deleting
  G4AutoDelete::Register( m_magneticFieldMessenger );

  // Make "sensitive detectors" for the EM liquid argon layers
  auto EMArgonDetector = new EnergyCounter( "homogeneous",1);
  G4SDManager::GetSDMpointer()->AddNewDetector( EMArgonDetector );
  this->SetSensitiveDetector( "homogeneous", EMArgonDetector );

  // Make "sensitive detectors" for the Hadronic liquid argon layers
  for ( unsigned int layerIndex = 1; layerIndex <= nLayers; ++layerIndex )
  {
    std::string detectorName = "Detector" + std::to_string( layerIndex );

    auto hadronicArgonDetector = new EnergyCounter( detectorName, layerIndex+1 );
    G4SDManager::GetSDMpointer()->AddNewDetector( hadronicArgonDetector );
    this->SetSensitiveDetector( detectorName, hadronicArgonDetector );
  }
}
