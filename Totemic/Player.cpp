#include <Thor\Time\StopWatch.hpp>
#include <Thor\Animation\FrameAnimation.hpp>
#include <Thor\Math\Random.hpp>
#include <iostream>
#include <Box2D\Dynamics\b2World.h>

#include "Player.h"
#include "Gatherer.h"
#include "Defender.h"
#include "ResourceHolder.h"
#include "Box2D\Dynamics\b2Body.h"
#include "PhysicsHelper.h"
#include "Config.h"
#include "Math.h"
#include "FloatingScoreText.h"
#include "Level.h"
#include "PlayState.h"

Player::Player()
{
	m_bounty = 0;
	m_hotspotScoreSum = 0.f;
	m_points = 0.f;
	m_online = true;
	m_dying = false;
	m_shield = false;
	m_isTweeningTotem = false;
	m_currentTotemTween = nullptr;
	m_holdingTotem = false;
	m_won = false;
	m_dead = false;
	m_stunned = false;
	m_respawnProtection = false;
	m_addedScoreTextTween = false;
	m_stunnedTimer.reset();
	m_shieldTimer.reset();
	m_postCheckDead = false;
	m_changingOrder = false;
	m_defender = nullptr;
	m_gatherer = nullptr;
	m_tweeningValue = 0.f;
	m_timer = new thor::StopWatch();
	m_deathTimer = new thor::CallbackTimer();
	m_deathTimer->connect(std::bind(&Player::onRespawn, this, std::placeholders::_1));
	m_totemSprite = new sf::Sprite();
	m_totemBountyIcon = new sf::Sprite();
	m_totemBountyIconAnimator = new thor::Animator<sf::Sprite, std::string>;
	m_totemBountyAmount = new sf::Text();
	mWinScoreText = new sf::Text();
	m_winNumberSprite = new sf::Sprite();
}
Player::~Player()
{
	delete m_gatherer;
	m_gatherer = nullptr;

	delete m_defender;
	m_defender = nullptr;

	delete m_deathTimer;
	m_deathTimer = nullptr;

	delete m_totemSprite;
	m_totemSprite = nullptr;

	delete m_barPointsIndicator;
	m_barPointsIndicator = nullptr;

	delete m_timer;
	m_timer = nullptr;

	delete m_totemBountyAmount;
	m_totemBountyAmount = nullptr;

	delete m_totemBountyIcon;
	m_totemBountyIcon = nullptr;

	delete m_totemBountyIconAnimator;
	m_totemBountyIconAnimator = nullptr;

	delete m_totemBountyAnimation;
	m_totemBountyAnimation = nullptr;

	delete mWinScoreText;
	mWinScoreText = nullptr;

	delete m_winNumberSprite;
	m_winNumberSprite = nullptr;
}

void Player::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	//override states if game is won and on scoreboard
	if (game->m_gameWon)
	{
		target.draw(*m_defender->getSprite());
		target.draw(*m_gatherer->getSprite());
	} 
	else if (m_dying)
	{
		target.draw(*m_defender->getDeathSprite());
		target.draw(*m_gatherer->getDeathSprite());
	} else if (!m_dead)
	{
		target.draw(*m_defender->getSprite());
		target.draw(*m_gatherer->getSprite());
		if (m_shield)
			target.draw(*m_gatherer->m_shieldOverlay);
		if (m_stunned)
		{
			target.draw(*m_defender->m_stunBirds);
		}
	}

	target.draw(*mWinScoreText);
}
void Player::setDevice(unsigned int p_deviceNo)
{
	m_deviceNo = p_deviceNo;
}
void Player::clear(b2World& world)
{
	if (m_defender != nullptr)
	{
		world.DestroyBody(m_defender->getBody());
		delete m_defender;
		m_defender = nullptr;
	}

	if (m_gatherer != nullptr)
	{
		world.DestroyBody(m_gatherer->getBody());
		delete m_gatherer;
		m_gatherer = nullptr;
	}

	m_dead = false;
	m_won = false;
	m_points = 0.f;
}
void Player::setDefender(Defender* p_defender)
{
	m_defender = p_defender;
	m_defender->setPlayer(this);
	m_defender->setType(DEFENDER);
	m_defender->getSprite()->setPosition(m_defender->getSpawnPosition());
	m_defender->getSprite()->setOrigin(128.f, 128.f);

	m_defender->getDeathSprite()->setPosition(m_defender->getSpawnPosition());
	m_defender->getDeathSprite()->setOrigin(128, 128);

	m_defender->m_stunBirds->setOrigin(128, 128);
	
	thor::FrameAnimation* walk_animation = new thor::FrameAnimation();
	walk_animation->addFrame(1.f, sf::IntRect(0,   0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(257, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(514, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(771, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(1028, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(1285, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(1542, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(1799, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(2056, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(2313, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(2570, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(2827, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(3084, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(3341, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(3598, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(3855, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(4112, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(4369, 0, 256, 256));
	walk_animation->addFrame(1.f, sf::IntRect(4626, 0, 256, 256));

	thor::FrameAnimation* death_animation = new thor::FrameAnimation();
	death_animation->addFrame(1.f, sf::IntRect(0, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(257, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(514, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(771, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(1028, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(1285, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(1542, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(1799, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(2056, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(2313, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(2570, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(2827, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(3084, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(3341, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(3598, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(3855, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(4112, 0, 256, 256));
	death_animation->addFrame(1.f, sf::IntRect(4369, 0, 256, 256));

	m_defender->addAnimation("Walk_Animation", walk_animation);
	m_defender->addAnimation("Death_Animation", death_animation);

	m_defender->getDeathAnimator()->addAnimation("death", *death_animation, sf::seconds(DEFENDER_DEATH_ANIM_DURATION));
	m_defender->getAnimatior()->addAnimation("walk", *walk_animation, sf::seconds(DEFENDER_WALK_ANIM_DURATION));
	m_defender->getAnimatior()->playAnimation("walk", true);
}
void Player::setGatherer(Gatherer* p_gatherer)
{
	m_gatherer = p_gatherer;

	m_gatherer->setPlayer(this);
	m_gatherer->setType(GATHERER);
	m_gatherer->getSprite()->setPosition(m_gatherer->getSpawnPosition());
	m_gatherer->getSprite()->setOrigin(64.f, 64.f);

	m_gatherer->getDeathSprite()->setPosition(m_gatherer->getSpawnPosition());
	m_gatherer->getDeathSprite()->setOrigin(55, 185);

	m_gatherer->m_shieldOverlay->setPosition(m_gatherer->getSpawnPosition());
	m_gatherer->m_shieldOverlay->setOrigin(73, 69);
	
	thor::FrameAnimation* walk_animation = new thor::FrameAnimation();
	walk_animation->addFrame(1.f, sf::IntRect(0, 0, 128, 128));
	walk_animation->addFrame(1.f, sf::IntRect(128, 0, 128, 128));
	walk_animation->addFrame(1.f, sf::IntRect(256, 0, 128, 128));
	walk_animation->addFrame(1.f, sf::IntRect(385, 0, 128, 128));
	walk_animation->addFrame(1.f, sf::IntRect(512, 0, 128, 128));

	thor::FrameAnimation* stun_animation = new thor::FrameAnimation();
	stun_animation->addFrame(1.f, sf::IntRect(0, 129, 128, 128));
	stun_animation->addFrame(1.f, sf::IntRect(128, 129, 128, 128));
	stun_animation->addFrame(1.f, sf::IntRect(256, 129, 128, 128));
	stun_animation->addFrame(1.f, sf::IntRect(385, 129, 128, 128));
	stun_animation->addFrame(1.f, sf::IntRect(512, 129, 128, 128));

	int width = 272;
	thor::FrameAnimation* death_animation = new thor::FrameAnimation();
	death_animation->addFrame(1.f, sf::IntRect(width * 0, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 1, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 2, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 3, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 4, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 5, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 6, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 7, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 8, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 9, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 10, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 11, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 12, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 13, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 14, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 15, 0, width, width));
	death_animation->addFrame(1.f, sf::IntRect(width * 16, 0, width, width));

	int size = 128;
	thor::FrameAnimation* walkGlowAnim = new thor::FrameAnimation();
	walkGlowAnim->addFrame(1.f, sf::IntRect(size * 0, 258, size, size));
	walkGlowAnim->addFrame(1.f, sf::IntRect(size * 1, 258, size, size));
	walkGlowAnim->addFrame(1.f, sf::IntRect(size * 2, 258, size, size));
	walkGlowAnim->addFrame(1.f, sf::IntRect(size * 3, 258, size, size));
	walkGlowAnim->addFrame(1.f, sf::IntRect(size * 4, 258, size, size));

	thor::FrameAnimation* stunGlowAnim = new thor::FrameAnimation();
	stunGlowAnim->addFrame(1.f, sf::IntRect(size * 0, 387, size, size));
	stunGlowAnim->addFrame(1.f, sf::IntRect(size * 1, 387, size, size));
	stunGlowAnim->addFrame(1.f, sf::IntRect(size * 2, 387, size, size));
	stunGlowAnim->addFrame(1.f, sf::IntRect(size * 3, 387, size, size));
	stunGlowAnim->addFrame(1.f, sf::IntRect(size * 4, 387, size, size));

	m_gatherer->addAnimation("Death_Animation", death_animation);
	m_gatherer->addAnimation("Walk_Animation", walk_animation);
	m_gatherer->addAnimation("Stun_Animation", stun_animation);
	m_gatherer->addAnimation("Walk_Glow_Animation", walkGlowAnim);
	m_gatherer->addAnimation("Stun_Glow_Animation", stunGlowAnim);

	m_gatherer->getDeathAnimator()->addAnimation("death", *death_animation, sf::seconds(GATHERER_DEATH_ANIM_DURATION));
	m_gatherer->getAnimatior()->addAnimation("walk", *walk_animation, sf::seconds(GATHERER_WALK_ANIM_DURATION));
	m_gatherer->getAnimatior()->addAnimation("stun", *stun_animation, sf::seconds(0.6f));
	m_gatherer->getAnimatior()->addAnimation("walk_glow", *walkGlowAnim, sf::seconds(GATHERER_WALK_ANIM_DURATION));
	m_gatherer->getAnimatior()->addAnimation("stun_glow", *stunGlowAnim, sf::seconds(0.6f));
	m_gatherer->getAnimatior()->playAnimation("walk", true);
}
void Player::setDead(bool value)
{
	m_dead = value;
	m_postCheckDead = true;
	m_respawnProtectionTimer.reset();
}
void Player::setDying(bool value)
{
	m_dying = value;
	if (m_dying)
	{
		m_gatherer->getDeathSprite()->setPosition(m_gatherer->getSprite()->getPosition());
		m_gatherer->getDeathAnimator()->playAnimation("death", false);
		m_defender->getDeathSprite()->setPosition(m_defender->getSprite()->getPosition());
		m_defender->getDeathAnimator()->playAnimation("death", false);
		m_postCheckDead = true;
		game->addDeathcloud(m_gatherer->getSprite()->getPosition(), m_deathCloudTextureRect);
		
	}
}
void Player::setColor(sf::Color color)
{
	m_color = color;
}
void Player::setOrder(unsigned int index)
{
	m_order = index;
}
void Player::setChangingOrder(bool value)
{
	m_changingOrder = value;
}
void Player::addPoints(float value, sf::Vector2f position, PlayerScoreTypes type)
{
	m_points += value;
	if (m_points >= POINTS_TO_WIN)
	{
		m_won = true;
		m_points = POINTS_TO_WIN;
	}

	if (type == SCORE_HOTSPOT)
	{
		if (m_hotspotScoreSum >= FLOATING_SCORE_HOTSPOT_SUM)
		{
			m_hotspotScoreSum = FLOATING_SCORE_HOTSPOT_SUM - m_hotspotScoreSum;
			FloatingScoreText* FST = new FloatingScoreText();
			FST->getText()->setPosition(position);
			FST->getText()->setFont(m_resourceHolder->getFont("lithospro.otf"));

			FST->getText()->setString("+" + std::to_string(static_cast<int>(FLOATING_SCORE_HOTSPOT_SUM)));
			FST->setScore(FLOATING_SCORE_HOTSPOT_SUM);
			m_floatingScoreTexts->push_back(FST);
		}
		else
		{
			m_hotspotScoreSum += value;
		}
	}
	else
	{
		FloatingScoreText* FST = new FloatingScoreText();
		FST->getText()->setPosition(position);
		FST->getText()->setFont(m_resourceHolder->getFont("lithospro.otf"));

		FST->getText()->setString("+" + std::to_string(static_cast<int>(value)));
		FST->setScore(value);
		m_floatingScoreTexts->push_back(FST);
	}
}
void Player::setPointsBarImage(std::string value)
{
	m_pointsBarImage = value;
}
void Player::setPointsIndicator(sf::Sprite* sprite)
{
	m_barPointsIndicator = sprite;
}
void Player::setFSTRef(std::vector<FloatingScoreText*>& FSTRef)
{
	m_floatingScoreTexts = &FSTRef;
}
void Player::setResourceHolder(ResourceHolder* resourceHolder)
{
	m_resourceHolder = resourceHolder;
}
void Player::processEventualDeath(Level* level)
{
	if (!m_postCheckDead) return;
	if (m_dying)
	{
		m_bounty = 0;
		
		m_gatherer->getBody()->SetActive(false);
		m_defender->getBody()->SetActive(false);
	}
	if (m_dead)
	{
		/*std::vector<PlayerSpawn*> playerSpawns = level->getPlayerSpawns();
		int randomSpawnIndex = thor::random(0U, playerSpawns.size() - 1);
		thor::StopWatch bandAid; bandAid.restart();
		while (playerSpawns[randomSpawnIndex]->occupied == true)
		{
			randomSpawnIndex = thor::random(0U, playerSpawns.size() - 1);
			if (bandAid.getElapsedTime() > sf::seconds(0.1f))
			{
				std::cout << "A bandaid was used :)" << std::endl;
				break;
			}
		}*/
		/*level->setPlayerSpawnOccupied(randomSpawnIndex, true);
		m_gatherer->getBody()->SetTransform(PhysicsHelper::gameToPhysicsUnits(playerSpawns[randomSpawnIndex]->gat_spawn), m_gatherer->getBody()->GetAngle());
		m_gatherer->getBody()->SetLinearVelocity(b2Vec2(0.f, 0.f));
		m_gatherer->getBody()->SetAngularVelocity(0.f);

		m_defender->getBody()->SetTransform(PhysicsHelper::gameToPhysicsUnits(playerSpawns[randomSpawnIndex]->def_spawn), m_defender->getBody()->GetAngle());
		m_defender->getBody()->SetLinearVelocity(b2Vec2(0.f, 0.f));
		m_defender->getBody()->SetAngularVelocity(0.f);
		*/
		m_deathTimer->restart(sf::seconds(4.f));
	}
	m_postCheckDead = false;
}
thor::CallbackTimer* Player::getDeathTimer()
{
	return m_deathTimer;
}
sf::Sprite* Player::getTotemSprite()
{
	return m_totemSprite;
}
sf::Sprite* Player::getPointsIndicator()
{
	return m_barPointsIndicator;
}
Gatherer* Player::getGatherer()
{
	return m_gatherer;
}
Defender* Player::getDefender()
{
	return m_defender;
}
unsigned int Player::getDevice()
{
	return m_deviceNo;
}
thor::StopWatch* Player::getTimer()
{
	return m_timer;
}
sf::Color Player::getColor()
{
	return m_color;
}
unsigned int Player::getOrder()
{
	return m_order;
}
float Player::getPoints()
{
	return m_points;
}
std::string Player::getPointsBarImage()
{
	return m_pointsBarImage;
}
bool Player::isDead()
{
	return m_dead;
}
bool Player::isChangingOrder()
{
	return m_changingOrder;
}
bool Player::hasWon()
{
	return m_won;
}
bool Player::isDying()
{
	return m_dying;
}
void Player::onRespawn(thor::CallbackTimer& trigger)
{
	m_dead = false;
	m_dying = false;
	m_respawnProtection = true;
	m_respawnProtectionTimer.start();
	m_defender->getBody()->SetActive(true);
	m_gatherer->getBody()->SetActive(true);
	
	Level* level = game->m_currentLevel;
	std::vector<PlayerSpawn*> playerSpawns = level->getPlayerSpawns();
	int randomSpawnIndex = thor::random(0U, playerSpawns.size() - 1);
	thor::StopWatch bandAid; bandAid.restart();
	while (playerSpawns[randomSpawnIndex]->occupied == true)
	{
		randomSpawnIndex = thor::random(0U, playerSpawns.size() - 1);
		if (bandAid.getElapsedTime() > sf::seconds(0.5f))
		{
			std::cout << "A bandaid was used :)" << std::endl;
			break;
		}
	}
	level->setPlayerSpawnOccupied(randomSpawnIndex, true);
	m_gatherer->getBody()->SetTransform(PhysicsHelper::gameToPhysicsUnits(playerSpawns[randomSpawnIndex]->gat_spawn), m_gatherer->getBody()->GetAngle());
	m_gatherer->getBody()->SetLinearVelocity(b2Vec2(0.f, 0.f));
	m_gatherer->getBody()->SetAngularVelocity(0.f);

	m_defender->getBody()->SetTransform(PhysicsHelper::gameToPhysicsUnits(playerSpawns[randomSpawnIndex]->def_spawn), m_defender->getBody()->GetAngle());
	m_defender->getBody()->SetLinearVelocity(b2Vec2(0.f, 0.f));
	m_defender->getBody()->SetAngularVelocity(0.f);

}
void Player::setRespawnProtection(bool value)
{
	m_respawnProtection = value;
}
void Player::setStunned(bool value)
{
	m_stunned = value;
	if (m_stunned) m_stunnedTimer.start();
	if (!m_stunned) m_stunnedTimer.reset();
}
bool Player::isStunned()
{
	return m_stunned;
}
void Player::setShield(bool value)
{
	m_shield = value;
	if (m_shield)
		m_shieldTimer.restart();
}
void Player::addToBounty(int value)
{
	m_bounty += value;
	m_totemBountyAmount->setString("x" + std::to_string(m_bounty));
	m_totemBountyAmount->setOrigin(m_totemBountyAmount->getGlobalBounds().width / 2.f, 0);
}
void Player::resetBounty()
{
	m_bounty = 0;
}
bool Player::hasShield()
{
	return m_shield;
}
int Player::getBounty()
{
	return m_bounty;
}
thor::StopWatch Player::getRespawnProtectionTimer()
{
	return m_respawnProtectionTimer;
}
bool Player::isProtected()
{
	return m_respawnProtection;
}
void Player::setPoints(int val)
{
	m_points = val;
}