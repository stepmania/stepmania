#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: TrailUtil

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "TrailUtil.h"
#include "Trail.h"
#include "Course.h"
#include "XmlFile.h"
#include "GameManager.h"


void TrailID::FromTrail( const Trail *p )
{
	if( p == NULL )
	{
		st = STEPS_TYPE_INVALID;
		cd = COURSE_DIFFICULTY_INVALID;
	}
	else
	{
		st = p->m_StepsType;
		cd = p->m_CourseDifficulty;
	}
}

Trail *TrailID::ToTrail( const Course *p, bool bAllowNull ) const
{
	ASSERT( p );

	if( st == STEPS_TYPE_INVALID || cd == COURSE_DIFFICULTY_INVALID )
		return NULL;

	return p->GetTrail( st, cd );
}

XNode* TrailID::CreateNode() const
{
	XNode* pNode = new XNode;
	pNode->name = "Trail";

	pNode->AppendAttr( "StepsType", GameManager::NotesTypeToString(st) );
	pNode->AppendAttr( "CourseDifficulty", CourseDifficultyToString(cd) );

	return pNode;
}

void TrailID::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->name == "Trail" );

	CString sTemp;

	pNode->GetAttrValue("StepsType", sTemp);
	st = GameManager::StringToNotesType( sTemp );

	pNode->GetAttrValue("CourseDifficulty", sTemp);
	cd = StringToCourseDifficulty( sTemp );
}

CString TrailID::ToString() const
{
	CString s = GameManager::NotesTypeToString(st);
	s += " " + CourseDifficultyToString(cd);
	return s;
}

bool TrailID::IsValid() const
{
	return st != STEPS_TYPE_INVALID && cd != COURSE_DIFFICULTY_INVALID;
}

bool TrailID::operator<( const TrailID &rhs ) const
{
#define COMP(a) if(a<rhs.a) return true; if(a>rhs.a) return false;
	COMP(st);
	COMP(cd);
#undef COMP
	return false;
}
