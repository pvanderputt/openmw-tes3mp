
#include "aisequence.hpp"

#include "aipackage.hpp"

#include "aiwander.hpp"
#include "aiescort.hpp"
#include "aitravel.hpp"
#include "aifollow.hpp"
#include "aiactivate.hpp"
#include "aicombat.hpp"

#include "..\mwworld\class.hpp"
#include "creaturestats.hpp"
#include "npcstats.hpp"
#include "..\mwbase\environment.hpp"
#include "..\mwbase\world.hpp"
#include "..\mwworld\player.hpp"

void MWMechanics::AiSequence::copy (const AiSequence& sequence)
{
    for (std::list<AiPackage *>::const_iterator iter (sequence.mPackages.begin());
        iter!=sequence.mPackages.end(); ++iter)
        mPackages.push_back ((*iter)->clone());
    mCombat = sequence.mCombat;
    mCombatPackage = sequence.mCombatPackage;
}

MWMechanics::AiSequence::AiSequence() : mDone (false), mCombat (false), mCombatPackage (0) {}

MWMechanics::AiSequence::AiSequence (const AiSequence& sequence) : mDone (false)
{
    copy (sequence);
}

MWMechanics::AiSequence& MWMechanics::AiSequence::operator= (const AiSequence& sequence)
{
    if (this!=&sequence)
    {
        clear();
        copy (sequence);
    }
    
    return *this;
}

MWMechanics::AiSequence::~AiSequence()
{
    clear();
}

int MWMechanics::AiSequence::getTypeId() const
{
    if (mPackages.empty())
        return -1;
        
    return mPackages.front()->getTypeId();
}

bool MWMechanics::AiSequence::isPackageDone() const
{
    return mDone;
}

void MWMechanics::AiSequence::execute (const MWWorld::Ptr& actor)
{
    if(actor != MWBase::Environment::get().getWorld()->getPlayer().getPlayer())
    {
        if(mCombat)
        {
            //mCombatPackage->execute(actor);
        }
        else
        {
            //mCombat = true;
            //mCombatPackage = new AiCombat("player");

            /*if(actor != MWBase::Environment::get().getWorld()->getPlayer().getPlayer())
            {
            MWMechanics::DrawState_ state = MWWorld::Class::get(actor).getNpcStats(actor).getDrawState();
            if (state == MWMechanics::DrawState_Spell || state == MWMechanics::DrawState_Nothing)
            MWWorld::Class::get(actor).getNpcStats(actor).setDrawState(MWMechanics::DrawState_Weapon);    
            MWWorld::Class::get(actor).getCreatureStats(actor).setAttackingOrSpell(true);

            MWWorld::Ptr player = MWBase::Environment::get().getWorld()->getPlayer().getPlayer();
            ESM::Position pos = actor.getRefData().getPosition();
            const ESM::Pathgrid *pathgrid =
            MWBase::Environment::get().getWorld()->getStore().get<ESM::Pathgrid>().search(*actor.getCell()->mCell);

            int cellX = actor.getCell()->mCell->mData.mX;
            int cellY = actor.getCell()->mCell->mData.mY;
            float xCell = 0;
            float yCell = 0;

            if (actor.getCell()->mCell->isExterior())
            {
            xCell = actor.getCell()->mCell->mData.mX * ESM::Land::REAL_SIZE;
            yCell = actor.getCell()->mCell->mData.mY * ESM::Land::REAL_SIZE;
            }

            ESM::Pathgrid::Point dest;
            dest.mX = player.getRefData().getPosition().pos[0];
            dest.mY = player.getRefData().getPosition().pos[1];
            dest.mZ = player.getRefData().getPosition().pos[2];

            ESM::Pathgrid::Point start;
            start.mX = pos.pos[0];
            start.mY = pos.pos[1];
            start.mZ = pos.pos[2];

            PathFinder mPathFinder;
            mPathFinder.buildPath(start, dest, pathgrid, xCell, yCell, true);
            float zAngle = mPathFinder.getZAngleToNext(pos.pos[0], pos.pos[1]);
            MWBase::Environment::get().getWorld()->rotateObject(actor, 0, 0, zAngle, false);
            MWWorld::Class::get(actor).getMovementSettings(actor).mPosition[1] = 1;

            if(dest.mX - start.mX < 100)
            {
            MWWorld::Class::get(actor).getCreatureStats(actor).setAttackingOrSpell(false);
            }
            }*/
            if (!mPackages.empty())
            {
                if (mPackages.front()->execute (actor))
                {
                    mPackages.erase (mPackages.begin());
                    mDone = true;
                }
                else
                    mDone = false;    
            }
        }
    }
}

void MWMechanics::AiSequence::clear()
{
    for (std::list<AiPackage *>::const_iterator iter (mPackages.begin()); iter!=mPackages.end(); ++iter)
        delete *iter;
    
    if(mCombatPackage) delete mCombatPackage;
    mPackages.clear();
}

void MWMechanics::AiSequence::stack (const AiPackage& package)
{
    mPackages.push_front (package.clone());
}

void MWMechanics::AiSequence::queue (const AiPackage& package)
{
    mPackages.push_back (package.clone());
}

void MWMechanics::AiSequence::fill(const ESM::AIPackageList &list)
{
    for (std::vector<ESM::AIPackage>::const_iterator it = list.mList.begin(); it != list.mList.end(); ++it)
    {
        MWMechanics::AiPackage* package;
        if (it->mType == ESM::AI_Wander)
        {
            ESM::AIWander data = it->mWander;
            std::vector<int> idles;
            for (int i=0; i<8; ++i)
                idles.push_back(data.mIdle[i]);
            package = new MWMechanics::AiWander(data.mDistance, data.mDuration, data.mTimeOfDay, idles, data.mUnk);
        }
        else if (it->mType == ESM::AI_Escort)
        {
            ESM::AITarget data = it->mTarget;
            package = new MWMechanics::AiEscort(data.mId.toString(), data.mDuration, data.mX, data.mY, data.mZ);
        }
        else if (it->mType == ESM::AI_Travel)
        {
            ESM::AITravel data = it->mTravel;
            package = new MWMechanics::AiTravel(data.mX, data.mY, data.mZ);
        }
        else if (it->mType == ESM::AI_Activate)
        {
            ESM::AIActivate data = it->mActivate;
            package = new MWMechanics::AiActivate(data.mName.toString());
        }
        else //if (it->mType == ESM::AI_Follow)
        {
            ESM::AITarget data = it->mTarget;
            package = new MWMechanics::AiFollow(data.mId.toString(), data.mDuration, data.mX, data.mY, data.mZ);
        }
        mPackages.push_back(package);
    }
}
