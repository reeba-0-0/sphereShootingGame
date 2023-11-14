// assignmentFinalCopy.cpp: A program using the TL-Engine

#include <TL-Engine.h>
#include <cmath> //maths library for vectors
#include <sstream>

//create constants for any values that don't change
using namespace tle;
const float kCameraX = 0.0f;
const float kCameraY = 200.0f;
const float kCameraZ = -170.0f;

const float kRotationX = 45.0f;
const float kPlatformPosX = 0.0f;
const float kPlatformPosY = -5.0f;
const float kPlatformPosZ = 0.0f;

const float kSphereInitialX = 85.0f;
const float kSphereInitialY = 10.0f;
const float kSphereInitialZ = 85.0f;

const float kCubeInitialX = 0.0f;
const float kCubeInitialY = 5.0f;
const float kCubeInitialZ = 0.0f;

const float kBulletInitialX = 0.0f;
const float kBulletInitialY = 5.0f;
const float kBulletInitialZ = 0.0f;

const int kNumSpheres = 4;
const int kSphere0 = 0;
const int kSphere1 = 1;
const int kSphere2 = 2;
const int kSphere3 = 3;

const float kSphereBoundary = 85.0f;
const float kSphereInitialMovementSpeed = 0.01f;
const float kSphereInitialRotationSpeed = 0.05f;
float kSphereCurrentMovementSpeed = 0.01f;
float kSphereCurrentRotationSpeed = 0.05f;
const float kIncreaseSphereSpeed = 1.1f;
const float kDecreaseSphereSpeed = 0.9f;
const float kBulletSpeed = 5.0f;
const float kBulletMaxDistance = 200.0f;

const EKeyCode keyUp = Key_Up;
const EKeyCode keyDown = Key_Down;
const EKeyCode keyA = Key_A;
const EKeyCode keyD = Key_D;
const EKeyCode keyP = Key_P;
const EKeyCode keyR = Key_R;
const EKeyCode keySpace = Key_Space;
const EKeyCode keyEscape = Key_Escape;

const float kCentreFontX = 640.0f;
const float kCentreFontY = 320.0f;
const float kRightFontX = 1280.0f;
const float kFontSize = 30.0f;

const float kCubeBulletDistance = 200.0f;
const float kBulletSphereDistance = 10.0f;
const float kChangeInY = kSphereInitialY - kBulletInitialY;
const float kChangeInZ = kSphereInitialZ - kBulletInitialZ;
const float kDistanceFormula = sqrt(pow(kChangeInY, 2) + pow(kChangeInZ, 2));
const float kVectorLength = kDistanceFormula - 10;
const float kCollisionXValue = 5.0f;
const float kRemoveSphere = -20.0f;

const int kInitialSpheresHealth = 6;
const int kIncrementGameScore = 10;
const float kIncreaseRemainingSpheresSpeed = 1.25f;
const float kIncreaseRemainingSpheresRotation = 1.25f;
const int kSuperSpheresInitialHealth = 2;
const int kSphereAfterDamageHealth = 1;
const int kMaxGameScore = 60;
const int kDouble = 2;
const int kInitialBullets = 10;

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder( ".\\Media" );

	/**** Set up your scene here ****/
	ICamera* myCamera = myEngine->CreateCamera(kManual, kCameraX, kCameraY, kCameraZ);
	myCamera->RotateLocalX(kRotationX);


	IMesh* platformMesh = myEngine->LoadMesh("platform.x");
	IModel* platformModel = platformMesh->CreateModel(kPlatformPosX, kPlatformPosY, kPlatformPosZ);

	IMesh* spheresMesh = myEngine->LoadMesh("spheremesh.x");
	IModel* spheres[kNumSpheres];

	//use arrays to create spheres, for efficiency
	for (int i = 0; i < kNumSpheres; i++)
	{
		spheres[i] = spheresMesh->CreateModel();
	}

	//set their initial positions
	spheres[kSphere0]->SetPosition(-kSphereInitialX, kSphereInitialY, -kSphereInitialZ);
	spheres[kSphere1]->SetPosition(kSphereInitialX, kSphereInitialY, kSphereInitialZ);
	spheres[kSphere2]->SetPosition(-kSphereInitialX, kSphereInitialY, kSphereInitialZ);
	spheres[kSphere3]->SetPosition(kSphereInitialX, kSphereInitialY, -kSphereInitialZ);

	spheres[kSphere2]->SetSkin("super.jpg");
	spheres[kSphere3]->SetSkin("super.jpg");

	IMesh* cubeMesh = myEngine->LoadMesh("cubemesh.x");
	IModel* cubeModel = cubeMesh->CreateModel(kCubeInitialX, kCubeInitialY, kCubeInitialZ);

	IMesh* bulletMesh = myEngine->LoadMesh("bullet.x");
	IModel* bulletModel = bulletMesh->CreateModel(kBulletInitialX, kBulletInitialY, kBulletInitialZ);

	IFont* myFont = myEngine->LoadFont("Comic Sans MS", kFontSize);

	//arrays for spheres' coordinates
	float xCoordinates[kNumSpheres];
	float yCoordinates[kNumSpheres];
	float zCoordinates[kNumSpheres];

	float bulletZCoordinate;
	float bulletZCoordinateBoundary;

	//boolean to track if a bullet has been fired
	//initialise it to false as it isn't fired at the start of the game
	bool bulletFired = false;

	//track if game has ended, regardless of if player has won or lost
	//initialise to false
	bool endGame = false;

	//declare variables that are going to change during the game
	int bulletsLeft = 10;
	int initialGameScore = 0;
	int healthSphere2 = 2;
	int healthSphere3 = 2;

	//enums to track the state of the game, initialsed to playing until other conditions are met
	enum EGameStates { playing, paused, gameWon, gameOver };
	EGameStates gameState = playing;

	//enums to track the direction of the spheres, initialsed to clockwise until R is clicked
	enum EPathing { clockwise, antiClockwise };
	EPathing sphereDirection = clockwise;
	
	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		/*spheres' health is initialised to 6 as that's how many times you need to hit them to win
		use this to track if health has gone down,instead of incrementing score, 
		setting bulletFired to false and increasing speed for each sphere*/
		int spheresHealth = 6;

		//get x,y,z coordinates for each sphere
		for (int i = 0; i < kNumSpheres; i++)
		{
			xCoordinates[i] = spheres[i]->GetX();
			yCoordinates[i] = spheres[i]->GetY();
			zCoordinates[i] = spheres[i]->GetZ();
		}

		bulletZCoordinate = bulletModel->GetZ();

		//dynamic text for score, bullets left and speed 
		stringstream scoreText;
		scoreText << "Score: " << initialGameScore;
		
		myFont->Draw(scoreText.str(), 0, 0, kBlue);

		stringstream bulletsLeftText;
		bulletsLeftText << "Bullets left: " << bulletsLeft;
		myFont->Draw(bulletsLeftText.str(), kCentreFontX, 0, kBlue, kCentre);

		stringstream currentSpeedText;
		currentSpeedText << "Current speed: " << kSphereCurrentMovementSpeed;
		myFont->Draw(currentSpeedText.str(), kRightFontX, 0, kBlue, kRight);

		//game state is in playing mode when the spheres are moving
		if (gameState == playing)
		{
			for (int i = 0; i < kNumSpheres; i++) //loops through each sphere
			{
				if (sphereDirection == clockwise) // clockwise = forward, right, down, left
				{
					/*if x coordinates are less than or equal to -85 and z coordinates are less than or
					equal to 85*/
					if (xCoordinates[i] <= -kSphereBoundary && zCoordinates[i] <= kSphereBoundary)
					{
						spheres[i]->MoveZ(kSphereCurrentMovementSpeed); //spheres move forward
						spheres[i]->RotateX(kSphereCurrentRotationSpeed);//rotates spheres in the X axis so movement is natural

					}

					/*if x coordinates are less than or equal to -85 and z coordinates are more than 85*/
					else if (xCoordinates[i] <= kSphereBoundary && zCoordinates[i] > kSphereBoundary)
					{
						spheres[i]->MoveX(kSphereCurrentMovementSpeed); //spheres move right
						spheres[i]->RotateZ(-kSphereCurrentRotationSpeed);

					}

					/*if x coordinates are more than or equal to 85 and z coordinates are more than or
					equal to -85*/
					else if (xCoordinates[i] >= kSphereBoundary && zCoordinates[i] >= -kSphereBoundary)
					{
						spheres[i]->MoveZ(-kSphereCurrentMovementSpeed);//spheres move down
						spheres[i]->RotateX(-kSphereCurrentRotationSpeed);

					}

					/*if x coordinates are more than or equal to -85 and z coordinates are less than or
					equal to -85*/
					else if (xCoordinates[i] >= -kSphereBoundary && zCoordinates[i] <= -kSphereBoundary)
					{
						spheres[i]->MoveX(-kSphereCurrentMovementSpeed);//spheres move left
						spheres[i]->RotateZ(kSphereCurrentRotationSpeed);

					}
				}

				if (sphereDirection == antiClockwise) // anti-clockwise = down, right, forward, left
				{
					/*if x coordinates are less than or equal to -85 and z coordinates are more than or
					equal to -85*/
					if (xCoordinates[i] <= -kSphereBoundary && zCoordinates[i] > -kSphereBoundary)
					{
						spheres[i]->MoveZ(-kSphereCurrentMovementSpeed); // spheres moves down
						spheres[i]->RotateX(-kSphereCurrentRotationSpeed);
					}

					/*if x coordinates are less than or equal to 85 and z coordinates are less than or
					equal to 85*/
					else if (xCoordinates[i] <= kSphereBoundary && zCoordinates[i] <= kSphereBoundary)
					{
						spheres[i]->MoveX(kSphereCurrentMovementSpeed); // spheres move right
						spheres[i]->RotateZ(-kSphereCurrentRotationSpeed);
					}

					/*if x coordinates are more than or equal to 85 and z coordinates are less than or
					equal to 85*/
					else if (xCoordinates[i] >= kSphereBoundary && zCoordinates[i] <= kSphereBoundary)
					{
						spheres[i]->MoveZ(kSphereCurrentMovementSpeed); // spheres move forward
						spheres[i]->RotateX(kSphereCurrentRotationSpeed);
					}

					/*if x coordinates are less than or equal to -85 and z coordinates are less than or
					equal to 85*/
					else if (xCoordinates[i] >= -kSphereBoundary && zCoordinates[i] >= kSphereBoundary)
					{
						spheres[i]->MoveX(-kSphereCurrentMovementSpeed); //spheres move left
						spheres[i]->RotateZ(kSphereCurrentRotationSpeed);
					}
				}

			}
			//included within the playing game state
			if (myEngine->KeyHit(keyP))
			{
				gameState = paused;
			}
		}

		//(when P is clicked) if game is in playing mode, set it to paused
		//if game is in paused mode, set it to playing
		else if (gameState == paused)
		{
			myFont->Draw("Paused", kCentreFontX, kCentreFontY, kBlue, kCentre, kVCentre);
			if (myEngine->KeyHit(keyP))
			{
				gameState = playing;
			}
		}

		//(when R is clicked) if spheres are moving clockwise, move them anti-clockwise
		//if spheres are moving anti-clockwise, move them clockwisr
		if (myEngine->KeyHit(keyR))
		{
			if (sphereDirection == clockwise)
			{
				sphereDirection = antiClockwise;
			}

			else
			{
				sphereDirection = clockwise;
			}
		}

		if (myEngine->KeyHit(keyD))
		{
			// making sure speeds do not exceed 2 times the initial value
			//using speed's initial value so it doesn't change
			if (kSphereCurrentMovementSpeed < (kSphereInitialMovementSpeed * kDouble))
			{
				kSphereCurrentMovementSpeed *= kIncreaseSphereSpeed; // increase speed by 10%
				kSphereCurrentRotationSpeed *= kIncreaseSphereSpeed; // increase rotation by 10%
			}
		}

		else if (myEngine->KeyHit(keyA))
		{
			// making sure speeds are not less than half the initial value
			if (kSphereCurrentMovementSpeed > (kSphereInitialMovementSpeed / kDouble))
			{
				kSphereCurrentMovementSpeed *= kDecreaseSphereSpeed; // decrease speed by 10%
				kSphereCurrentRotationSpeed *= kDecreaseSphereSpeed; // decrease rotation by 10%
			}
		}


		//fire bullet if up key is hit, bool set to true
		if (myEngine->KeyHit(keyUp))
		{
			bulletFired = true;
		}

		//only executed if bulletFired = true (if conditions are met)
		if (bulletFired)
		{
			bulletModel->MoveZ(kSphereCurrentMovementSpeed * kBulletSpeed); // bullet speed is 5 times the current sphere speed
		}

		// if the bullet is more than or equal to 200 away from the cube, position it back inside
		//decrement the bullets left
		if (bulletZCoordinate >= kCubeBulletDistance)
		{
			bulletFired = false;
			bulletsLeft--;
			bulletModel->SetPosition(kBulletInitialX, kBulletInitialY, kBulletInitialZ);
		}

		//execute this code when a sphere is hit
		for (int i = 0; i < kNumSpheres; i++)
		{
			/*sphere is hit when the distance between the bullet and sphere is less than or
			equal to 10, the spheres' x coordinates are more than -5 and less than 5, their 
			z coordinates are more than or equal to 85 and y coordinates are more than 0*/
			if (bulletZCoordinate >= kVectorLength && xCoordinates[i] > -kCollisionXValue && xCoordinates[i] < kCollisionXValue && zCoordinates[i] >= kSphereInitialZ && yCoordinates[i] > 0)
			{
				if (i < kSphere2) //for sphere 0 and sphere 1 (regular spheres)
				{
					spheres[i]->SetY(kRemoveSphere); //remove sphere after 1 hit
					spheresHealth--; //decrement health for all spheres
				}
				// work with each super sphere separately to check if it's been hit once or twice
				else if (i == kSphere2) 
				{

					if (healthSphere2 == kSuperSpheresInitialHealth) //if health = 2 (initial value)
					{
						spheres[i]->SetSkin("regular.jpg"); //set skin to irregular after 1 hit
						spheresHealth--;
						healthSphere2--; //decrement health for sphere 3 when it's been hit once

					}
					else if (healthSphere2 == kSphereAfterDamageHealth) //if health = 1
					{
						spheres[i]->SetY(kRemoveSphere); //remove sphere after 2 hits
						spheresHealth--; //decrement health when it's been hit twice
					}
				}
				//repeat for the other super sphere
				else if (i == kSphere3)
				{
					if (healthSphere3 == kSuperSpheresInitialHealth)
					{
						spheres[i]->SetSkin("regular.jpg");
						spheresHealth--;
						healthSphere3--;

					}
					else if (healthSphere3 == kSphereAfterDamageHealth)
					{
						spheres[i]->SetY(kRemoveSphere);
						spheresHealth--;
					}
				}
				break;
			}
		}

		//if health has gone down (a sphere has been hit) then execute this code
		if (spheresHealth < kInitialSpheresHealth) //if health is less than 6 
		{
			initialGameScore += kIncrementGameScore; //add 10 points
			kSphereCurrentMovementSpeed *= kIncreaseRemainingSpheresSpeed; //increase speed by 25%
			kSphereCurrentRotationSpeed *= kIncreaseRemainingSpheresRotation; // increase rotation by 25%
			bulletFired = false; //set bullet fired to false
			bulletsLeft--; //decrement bullets
			bulletModel->SetPosition(kBulletInitialX, kBulletInitialY, kBulletInitialZ); //set bullets to initial position
		}
		
		//if the player scores 60 points, they've won and the game ends
		if (initialGameScore == kMaxGameScore)
		{
			gameState = gameWon;
			endGame = true;
			myFont->Draw("You win! Press Space To Restart", kCentreFontX, kCentreFontY, kYellow, kCentre, kVCentre);
		}
		 
		//if there are no bullets left then the player has lost and the endGame code is executed
		else if (bulletsLeft == 0)
		{
			gameState = gameOver;
			endGame = true;
			myFont->Draw("Game over. Press Space To Restart", kCentreFontX, kCentreFontY, kYellow, kCentre, kVCentre);
		}

		//code is executed when player loses or wins
		if (endGame)
		{
			for (int i = 0; i < kNumSpheres; i++)
			{
				spheres[i]->SetY(kRemoveSphere);
			}
			bulletFired = false; //can no longer fire bullets until space key is hit
		}

		//restart game after space key is hit
		if (myEngine->KeyHit(keySpace))
		{
			if (gameState == gameWon || gameState == gameOver)
			{
				//set spheres' back to initial positions
				spheres[0]->SetPosition(-kSphereInitialX, kSphereInitialY, -kSphereInitialZ);
				spheres[kSphere1]->SetPosition(kSphereInitialX, kSphereInitialY, kSphereInitialZ);
				spheres[kSphere2]->SetPosition(-kSphereInitialX, kSphereInitialY, kSphereInitialZ);
				spheres[kSphere3]->SetPosition(kSphereInitialX, kSphereInitialY, -kSphereInitialZ);

				//super spheres go back to having super skin
				spheres[kSphere2]->SetSkin("super.jpg");
				spheres[kSphere3]->SetSkin("super.jpg");

				//super spheres' health is back at 2 and all spheres' health is 6
				healthSphere2 = kSuperSpheresInitialHealth;
				healthSphere3 = kSuperSpheresInitialHealth;
				spheresHealth = kInitialSpheresHealth; 

				bulletsLeft = kInitialBullets; //reset bullets to 10 
				initialGameScore = 0; //reset score
				endGame = false; //spheres are now visible and bullets can be fired
				gameState = playing; //spheres move
				sphereDirection = clockwise; //reset direction to clockwise
				kSphereCurrentMovementSpeed = kSphereInitialMovementSpeed; //reset to initial sphere speed
				kSphereCurrentRotationSpeed = kSphereInitialRotationSpeed; //reset to initial rotation speed
			}
		}

		//player can choose to leave game whilst playing
		if (myEngine->KeyHit(keyEscape))
		{
			myEngine->Stop();
		}

	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
