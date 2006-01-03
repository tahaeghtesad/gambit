//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of supports for extensive forms
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "libgambit.h"

template <class T> void RemoveRedundancies(gbtList<T> &p_list)
{
  int i = 1; int j = 2;		
  while (i < p_list.Length()) {
    if (p_list[i] == p_list[j])
      p_list.Remove(j);
    else 
      j++;
    if (j > p_list.Length()) { i++; j = i+1; }
  }
}

class gbtEfgSupportInfoset   {
  friend class gbtEfgSupportPlayer;
protected:
  gbtArray<Gambit::GameAction> acts;

public:
  gbtEfgSupportInfoset(Gambit::GameInfoset);
  gbtEfgSupportInfoset ( const gbtEfgSupportInfoset &a);
  virtual ~gbtEfgSupportInfoset();
  gbtEfgSupportInfoset &operator=( const gbtEfgSupportInfoset &a);
  bool operator==( const gbtEfgSupportInfoset &a) const;
  inline const Gambit::GameAction operator[](const int &i) const { return acts[i]; }

  // Information
  inline const int Length() const { return acts.Length(); }
};

//----------------------------------------------------
// gbtEfgSupportInfoset: Constructors, Destructor, operators
// ---------------------------------------------------

gbtEfgSupportInfoset::gbtEfgSupportInfoset(Gambit::GameInfoset p_infoset)
{
  for (int i = 1; i <= p_infoset->NumActions(); i++) {
    acts.Append(p_infoset->GetAction(i));
  }
 }

gbtEfgSupportInfoset::gbtEfgSupportInfoset(const gbtEfgSupportInfoset &a)
  : acts(a.acts)
{ }

gbtEfgSupportInfoset::~gbtEfgSupportInfoset ()
{ }

gbtEfgSupportInfoset &gbtEfgSupportInfoset::operator=( const gbtEfgSupportInfoset &a)
{
  acts = a.acts; 
  return *this;
}

bool gbtEfgSupportInfoset::operator==(const gbtEfgSupportInfoset &a) const
{
  return (acts == a.acts);
}

class gbtEfgSupportPlayer{

protected:
  Gambit::GamePlayer efp;
  gbtArray < gbtEfgSupportInfoset *> infosets;
public:
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

//  gbtEfgSupportPlayer();
  gbtEfgSupportPlayer(const gbtEfgSupportPlayer &);
  gbtEfgSupportPlayer(Gambit::GamePlayer);
  virtual ~gbtEfgSupportPlayer();

  gbtEfgSupportPlayer &operator=(const gbtEfgSupportPlayer &);
  bool operator==(const gbtEfgSupportPlayer &s) const;

  //--------------------
  // Member Functions
  //--------------------

  // Append an action to an infoset;
  void AddAction(int iset, const Gambit::GameAction &);

  // Insert an action in a particular place in an infoset;
  void AddAction(int iset, const Gambit::GameAction &, int index);


  // Remove an action at int i, returns the removed action pointer
  Gambit::GameAction RemoveAction(int iset, int i);

  // Remove an action from an infoset . 
  // Returns true if the action was successfully removed, false otherwise.
  bool RemoveAction(int iset, Gambit::GameAction);

  // Get a garray of the actions in an Infoset
  const gbtArray<Gambit::GameAction> &ActionList(int iset) const
     { return infosets[iset]->acts; }

  // Get the gbtEfgSupportInfoset of an iset
  const gbtEfgSupportInfoset *ActionArray(int iset) const
     { return infosets[iset]; }

  // Get the gbtEfgSupportInfoset of an Infoset
  const gbtEfgSupportInfoset *ActionArray(const Gambit::GameInfoset &i) const
     { return infosets[i->GetNumber()]; }
  
  // Get an Action
  Gambit::GameAction GetAction(int iset, int index);

  // returns the index of the action if it is in the ActionSet
  int Find(const Gambit::GameAction &) const;
  int Find(int, Gambit::GameAction) const;

  // Number of Actions in a particular infoset
  int NumActions(int iset) const;

  // return the Gambit::GamePlayer of the gbtEfgSupportPlayer
  Gambit::GamePlayer GetPlayer(void) const;

  // checks for a valid gbtEfgSupportPlayer
  bool HasActiveActionsAtAllInfosets(void) const;
  bool HasActiveActionAt(const int &iset) const;

};

//--------------------------------------------------
// gbtEfgSupportPlayer: Constructors, Destructor, operators
//--------------------------------------------------

gbtEfgSupportPlayer::gbtEfgSupportPlayer(Gambit::GamePlayer p)
  : infosets(p->NumInfosets())
{
  efp = p;
  for (int i = 1; i <= p->NumInfosets(); i++) {
    infosets[i] = new gbtEfgSupportInfoset(p->GetInfoset(i));
  }
}

gbtEfgSupportPlayer::gbtEfgSupportPlayer( const gbtEfgSupportPlayer &s )
: infosets(s.infosets.Length())
{
  efp = s.efp;
  for (int i = 1; i <= s.infosets.Length(); i++){
    infosets[i] = new gbtEfgSupportInfoset(*(s.infosets[i]));
  }
}

gbtEfgSupportPlayer::~gbtEfgSupportPlayer()
{ 
  for (int i = 1; i <= infosets.Length(); i++)
    delete infosets[i];
}

gbtEfgSupportPlayer &gbtEfgSupportPlayer::operator=(const gbtEfgSupportPlayer &s)
{
  if (this != &s && efp == s.efp) {
    for (int i = 1; i<= infosets.Length(); i++)  {
      delete infosets[i];
      infosets[i] = new gbtEfgSupportInfoset(*(s.infosets[i]));
    }
  }    
  return *this;
}

bool gbtEfgSupportPlayer::operator==(const gbtEfgSupportPlayer &s) const
{
  if (infosets.Length() != s.infosets.Length() ||
      efp != s.efp)
    return false;
  
  int i;
  for (i = 1; i <= infosets.Length() && 
       *(infosets[i]) == *(s.infosets[i]);  i++);
  return (i > infosets.Length());
}

//------------------------------------------
// gbtEfgSupportPlayer: Member functions 
//------------------------------------------

// Append an action to a particular infoset;
void gbtEfgSupportPlayer::AddAction(int iset, const Gambit::GameAction &s)
{ 
  if (infosets[iset]->acts.Find(s))
    return;

  if (infosets[iset]->acts.Length() == 0) {
    infosets[iset]->acts.Append(s); 
  }
  else {
    int index = 1;
    while (index <= infosets[iset]->acts.Length() &&
	   infosets[iset]->acts[index]->GetNumber() < s->GetNumber()) 
      index++;
    infosets[iset]->acts.Insert(s,index);
  }
}

// Insert an action  to a particular infoset at a particular place;
void gbtEfgSupportPlayer::AddAction(int iset, const Gambit::GameAction &s, int index)
{ 
  if (!infosets[iset]->acts.Find(s))
    infosets[iset]->acts.Insert(s,index); 
}

// Remove an action from infoset iset at int i, 
// returns the removed Infoset pointer
Gambit::GameAction gbtEfgSupportPlayer::RemoveAction(int iset, int i) 
{ 
  return (infosets[iset]->acts.Remove(i)); 
}

// Removes an action from infoset iset . Returns true if the 
//Action was successfully removed, false otherwise.
bool gbtEfgSupportPlayer::RemoveAction(int  iset, Gambit::GameAction s )
{ 
  int t = infosets[iset]->acts.Find(s); 
  if (t>0) infosets[iset]->acts.Remove(t); 
  return (t>0); 
} 

// Get an action
Gambit::GameAction gbtEfgSupportPlayer::GetAction(int iset, int index)
{
  return (infosets[iset]->acts)[index];
}

// Number of Actions in a particular infoset
int gbtEfgSupportPlayer::NumActions(int iset) const
{
  return (infosets[iset]->acts.Length());
}

// Return the Gambit::GamePlayer of this gbtEfgSupportPlayer
Gambit::GamePlayer gbtEfgSupportPlayer::GetPlayer(void) const
{
  return efp;
}

int gbtEfgSupportPlayer::Find(const Gambit::GameAction &a) const
{
  return (infosets[a->GetInfoset()->GetNumber()]->acts.Find(a));
}

int gbtEfgSupportPlayer::Find(int p_infoset, Gambit::GameAction a) const
{
  return (infosets[p_infoset]->acts.Find(a));
}

// checks for a valid gbtEfgSupportPlayer
bool gbtEfgSupportPlayer::HasActiveActionsAtAllInfosets(void) const
{
  if (infosets.Length() != efp->NumInfosets())   return false;

  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->acts.Length() == 0)   return false;

  return true;
}

// checks for a valid gbtEfgSupportPlayer
bool gbtEfgSupportPlayer::HasActiveActionAt(const int &iset) const
{
  if (iset > efp->NumInfosets())   return false;

  if (infosets[iset]->acts.Length() == 0)   return false;

  return true;
}


//--------------------------------------------------
// gbtEfgSupport: Constructors, Destructors, Operators
//--------------------------------------------------

gbtEfgSupport::gbtEfgSupport(const Gambit::Game &p_efg) 
  : m_efg(p_efg), m_players(p_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl] = new gbtEfgSupportPlayer(p_efg->GetPlayer(pl));
  }
}

gbtEfgSupport::gbtEfgSupport(const gbtEfgSupport &p_support)
  : m_name(p_support.m_name), m_efg(p_support.m_efg),
    m_players(p_support.m_players.Length())
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    m_players[pl] = new gbtEfgSupportPlayer(*(p_support.m_players[pl]));
}

gbtEfgSupport::~gbtEfgSupport()
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    delete m_players[pl];
}

gbtEfgSupport &gbtEfgSupport::operator=(const gbtEfgSupport &p_support)
{
  if (this != &p_support && m_efg == p_support.m_efg) {
    m_name = p_support.m_name;
    for (int pl = 1; pl <= m_players.Length(); pl++)  {
      delete m_players[pl];
      m_players[pl] = new gbtEfgSupportPlayer(*(p_support.m_players[pl]));
    }
  }
  return *this;
}

bool gbtEfgSupport::operator==(const gbtEfgSupport &p_support) const
{
  if (m_players.Length() != p_support.m_players.Length())
    return false;

  int pl;
  for (pl = 1; (pl <= m_players.Length() &&
		*(m_players[pl]) == *(p_support.m_players[pl])); pl++);
  return (pl > m_players.Length());
}

bool gbtEfgSupport::operator!=(const gbtEfgSupport &p_support) const
{
  return !(*this == p_support);
}

//-----------------------------
// gbtEfgSupport: Member Functions 
//-----------------------------

int gbtEfgSupport::NumActions(int pl, int iset) const
{
  return m_players[pl]->NumActions(iset);
}

int gbtEfgSupport::NumActions(const Gambit::GameInfoset &i) const
{
  if (i->GetPlayer()->IsChance())
    return i->NumActions();
  else
    return m_players[i->GetPlayer()->GetNumber()]->NumActions(i->GetNumber());
}

const gbtArray<Gambit::GameAction> &gbtEfgSupport::Actions(int pl, int iset) const
{
  return m_players[pl]->ActionList(iset);
}

gbtArray<Gambit::GameAction> gbtEfgSupport::Actions(const Gambit::GameInfoset &i) const
{
  if (i->GetPlayer()->IsChance()) {
    gbtArray<Gambit::GameAction> actions;
    for (int act = 1; act <= i->NumActions(); act++) {
      actions.Append(i->GetAction(act));
    }
    return actions;
  }
  else
    return m_players[i->GetPlayer()->GetNumber()]->ActionList(i->GetNumber());
}

gbtList<Gambit::GameAction> gbtEfgSupport::ListOfActions(const Gambit::GameInfoset &i) const
{
  gbtArray<Gambit::GameAction> actions = Actions(i);
  gbtList<Gambit::GameAction> answer;
  for (int i = 1; i <= actions.Length(); i++)
    answer.Append(actions[i]);
  return answer;
}

int gbtEfgSupport::Find(const Gambit::GameAction &a) const
{
  if (a->GetInfoset()->GetGame() != m_efg)  assert(0);

  int pl = a->GetInfoset()->GetPlayer()->GetNumber();

  return m_players[pl]->Find(a);
}

int gbtEfgSupport::Find(int p_player, int p_infoset, Gambit::GameAction p_action) const
{
  return m_players[p_player]->Find(p_infoset, p_action);
}

bool gbtEfgSupport::ActionIsActive(Gambit::GameAction a) const
{
  //DEBUG
  //  if (a == NULL) { gout << "Action* is null.\n"; exit(0); }

  if (a->GetInfoset()->GetGame() != m_efg)   
    return false;

  int pl = a->GetInfoset()->GetPlayer()->GetNumber();

  if (pl == 0) return true; // Chance

  int act = m_players[pl]->Find(a);
  if (act == 0) 
    return false;
  else
    return true;
}

bool gbtEfgSupport::ActionIsActive(const int pl,
			       const int iset, 
			       const int act) const
{
  return 
    ActionIsActive(GetGame()->GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
}

bool 
gbtEfgSupport::AllActionsInSupportAtInfosetAreActive(const gbtEfgSupport &S,
						 const Gambit::GameInfoset &infset) const
{
  gbtArray<Gambit::GameAction> support_actions = S.Actions(infset);
  for (int i = 1; i <= support_actions.Length(); i++) {
    if (!ActionIsActive(support_actions[i]))
      return false;
  }
  return true;
}

bool gbtEfgSupport::HasActiveActionAt(const Gambit::GameInfoset &infoset) const
{
  if 
    ( !m_players[infoset->GetPlayer()->GetNumber()]->
      HasActiveActionAt(infoset->GetNumber()) )
    return false;

  return true;
}

int gbtEfgSupport::NumDegreesOfFreedom(void) const
{
  int answer(0);

  gbtList<Gambit::GameInfoset> active_infosets = ReachableInfosets(GetGame()->GetRoot());
  for (int i = 1; i <= active_infosets.Length(); i++)
    answer += NumActions(active_infosets[i]) - 1;

  return answer;  
}

bool gbtEfgSupport::HasActiveActionsAtAllInfosets(void) const
{
  if (m_players.Length() != m_efg->NumPlayers())   return false;
  for (int i = 1; i <= m_players.Length(); i++)
    if (!m_players[i]->HasActiveActionsAtAllInfosets())  return false;

  return true;
}

gbtPVector<int> gbtEfgSupport::NumActions(void) const
{
  gbtArray<int> foo(m_efg->NumPlayers());
  int i;
  for (i = 1; i <= m_efg->NumPlayers(); i++)
    foo[i] = m_players[i]->GetPlayer()->NumInfosets();

  gbtPVector<int> bar(foo);
  for (i = 1; i <= m_efg->NumPlayers(); i++)
    for (int j = 1; j <= m_players[i]->GetPlayer()->NumInfosets(); j++)
      bar(i, j) = NumActions(i,j);

  return bar;
}  

bool gbtEfgSupport::RemoveAction(const Gambit::GameAction &s)
{
  Gambit::GameInfoset infoset = s->GetInfoset();
  Gambit::GamePlayer player = infoset->GetPlayer();
 
  return m_players[player->GetNumber()]->RemoveAction(infoset->GetNumber(), s);
}

void gbtEfgSupport::AddAction(const Gambit::GameAction &s)
{
  Gambit::GameInfoset infoset = s->GetInfoset();
  Gambit::GamePlayer player = infoset->GetPlayer();

  m_players[player->GetNumber()]->AddAction(infoset->GetNumber(), s);
				      
}

int gbtEfgSupport::NumSequences(int j) const
{
  if (j < 1 || j > m_efg->NumPlayers()) return 1;
  gbtList<Gambit::GameInfoset> isets = ReachableInfosets(m_efg->GetPlayer(j));
  int num = 1;
  for(int i = 1; i <= isets.Length(); i++)
    num+=NumActions(isets[i]);
  return num;
}

int gbtEfgSupport::TotalNumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= m_efg->NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

gbtList<Gambit::GameNode> gbtEfgSupport::ReachableNonterminalNodes(const Gambit::GameNode &n) const
{
  gbtList<Gambit::GameNode> answer;
  if (!n->IsTerminal()) {
    const gbtArray<Gambit::GameAction> &actions = Actions(n->GetInfoset());
    for (int i = 1; i <= actions.Length(); i++) {
      Gambit::GameNode nn = n->GetChild(actions[i]->GetNumber());
      if (!nn->IsTerminal()) {
	answer.Append(nn);
	answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

gbtList<Gambit::GameNode> 
gbtEfgSupport::ReachableNonterminalNodes(const Gambit::GameNode &n,
					 const Gambit::GameAction &a) const
{
  gbtList<Gambit::GameNode> answer;
  Gambit::GameNode nn = n->GetChild(a->GetNumber());
  if (!nn->IsTerminal()) {
    answer.Append(nn);
    answer += ReachableNonterminalNodes(nn);
  }
  return answer;
}

gbtList<Gambit::GameInfoset> 
gbtEfgSupport::ReachableInfosets(const Gambit::GamePlayer &p) const
{ 
  gbtArray<Gambit::GameInfoset> isets;
  for (int iset = 1; iset <= p->NumInfosets(); iset++) {
    isets.Append(p->GetInfoset(iset));
  }
  gbtList<Gambit::GameInfoset> answer;

  for (int i = isets.First(); i <= isets.Last(); i++)
    if (MayReach(isets[i]))
      answer.Append(isets[i]);
  return answer;
}

gbtList<Gambit::GameInfoset> gbtEfgSupport::ReachableInfosets(const Gambit::GameNode &n) const
{
  gbtList<Gambit::GameInfoset> answer;
  gbtList<Gambit::GameNode> nodelist = ReachableNonterminalNodes(n);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer.Append(nodelist[i]->GetInfoset());
  RemoveRedundancies(answer);
  return answer;
}

gbtList<Gambit::GameInfoset> 
gbtEfgSupport::ReachableInfosets(const Gambit::GameNode &n, 
				 const Gambit::GameAction &a) const
{
  gbtList<Gambit::GameInfoset> answer;
  gbtList<Gambit::GameNode> nodelist = ReachableNonterminalNodes(n,a);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer.Append(nodelist[i]->GetInfoset());
  RemoveRedundancies(answer);
  return answer;
}

bool gbtEfgSupport::AlwaysReaches(const Gambit::GameInfoset &i) const
{
  return AlwaysReachesFrom(i, m_efg->GetRoot());
}

bool gbtEfgSupport::AlwaysReachesFrom(const Gambit::GameInfoset &i, const Gambit::GameNode &n) const
{
  if (n->IsTerminal()) return false;
  else
    if (n->GetInfoset() == i) return true;
    else {
      gbtArray<Gambit::GameAction> actions = Actions(n->GetInfoset());
      for (int j = 1; j <= actions.Length(); j++)
	if (!AlwaysReachesFrom(i,n->GetChild(actions[j]->GetNumber()))) 
	  return false;
    }
  return true;
}

bool gbtEfgSupport::MayReach(const Gambit::GameInfoset &i) const
{
  for (int j = 1; j <= i->NumMembers(); j++)
    if (MayReach(i->GetMember(j)))
      return true;
  return false;
}

bool gbtEfgSupport::MayReach(const Gambit::GameNode &n) const
{
  if (n == m_efg->GetRoot())
    return true;
  else {
    if (!ActionIsActive(n->GetPriorAction()))
      return false;
    else 
      return MayReach(n->GetParent());
  }
}


//----------------------------------------------------
//                gbtEfgSupportWithActiveInfo
// ---------------------------------------------------

// Utilities 
bool gbtEfgSupportWithActiveInfo::infoset_has_active_nodes(const int pl,
						       const int iset) const
{
  //DEBUG
  /*
  gout << "Got in with pl = " << pl << " and iset = " << iset << ".\n";
  if (InfosetIsActive(pl,iset))
    gout << "Apparently the infoset is active??\n";
  else
    gout << "Somehow it got deactivated.\n";
  */

  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    { //DEBUG
      /*
      gout << "With pl = " << pl << ", iset = " << iset
	   << ", and i = " << i << " the node is supposedly ";
      if (NodeIsActive(pl,iset,i))
	gout << "active.\n";
      else
	gout << "inactive.\n";
      */

    if (is_nonterminal_node_active[pl][iset][i])
      return true;
    }
  return false;
}

bool gbtEfgSupportWithActiveInfo::infoset_has_active_nodes(const Gambit::GameInfoset &i) const
{
  return infoset_has_active_nodes(i->GetPlayer()->GetNumber(), i->GetNumber());
}

void gbtEfgSupportWithActiveInfo::activate(const Gambit::GameNode &n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = true;
}

void gbtEfgSupportWithActiveInfo::deactivate(const Gambit::GameNode &n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = false;
}

void gbtEfgSupportWithActiveInfo::activate(const Gambit::GameInfoset &i)
{
  is_infoset_active[i->GetPlayer()->GetNumber()][i->GetNumber()] = true;
}

void gbtEfgSupportWithActiveInfo::deactivate(const Gambit::GameInfoset &i)
{
  is_infoset_active[i->GetPlayer()->GetNumber()][i->GetNumber()] = false;
}

void gbtEfgSupportWithActiveInfo::activate_this_and_lower_nodes(const Gambit::GameNode &n)
{
  if (!n->IsTerminal()) {
    activate(n); 
    activate(n->GetInfoset());
    gbtArray<Gambit::GameAction> actions(Actions(n->GetInfoset()));
    for (int i = 1; i <= actions.Length(); i++) 
      activate_this_and_lower_nodes(n->GetChild(actions[i]->GetNumber()));    
  }
}

void gbtEfgSupportWithActiveInfo::deactivate_this_and_lower_nodes(const Gambit::GameNode &n)
{
  if (!n->IsTerminal()) {  // THIS ALL LOOKS FISHY
    deactivate(n); 
    if ( !infoset_has_active_nodes(n->GetInfoset()) )
      deactivate(n->GetInfoset());
    gbtArray<Gambit::GameAction> actions(Actions(n->GetInfoset()));
      for (int i = 1; i <= actions.Length(); i++) 
	deactivate_this_and_lower_nodes(n->GetChild(actions[i]->GetNumber()));    
  }
}

void gbtEfgSupportWithActiveInfo::
deactivate_this_and_lower_nodes_returning_deactivated_infosets(const Gambit::GameNode &n, 
                                                gbtList<Gambit::GameInfoset> *list)
{
  if (!n->IsTerminal()) {
    deactivate(n); 
    if ( !infoset_has_active_nodes(n->GetInfoset()) ) {

      //DEBUG
      /*
      gout << "We are deactivating infoset " << n->GetInfoset()->GetNumber()
	   << " with support \n" << *this << "\n";
      */

      list->Append(n->GetInfoset()); 
      deactivate(n->GetInfoset());
    }
    gbtArray<Gambit::GameAction> actions(Actions(n->GetInfoset()));
      for (int i = 1; i <= actions.Length(); i++) 
	deactivate_this_and_lower_nodes_returning_deactivated_infosets(
			     n->GetChild(actions[i]->GetNumber()),list);    
  }
}

void gbtEfgSupportWithActiveInfo::InitializeActiveListsToAllActive()
{
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl); 
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active.Append(true);

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers(); n++)
	is_infosets_node_active.Append(true);
      is_players_node_active.Append(is_infosets_node_active);
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void gbtEfgSupportWithActiveInfo::InitializeActiveListsToAllInactive()
{
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl);
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active.Append(false);

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers()
; n++)
	is_infosets_node_active.Append(false);
      is_players_node_active.Append(is_infosets_node_active);
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void gbtEfgSupportWithActiveInfo::InitializeActiveLists()
{
  InitializeActiveListsToAllInactive();

  activate_this_and_lower_nodes(GetGame()->GetRoot());
}

// Constructors and Destructor
gbtEfgSupportWithActiveInfo::gbtEfgSupportWithActiveInfo(const Gambit::Game &E) 
  : gbtEfgSupport(E), 
    is_infoset_active(0,E->NumPlayers()), 
    is_nonterminal_node_active(0,E->NumPlayers())
{
  InitializeActiveLists();
}

gbtEfgSupportWithActiveInfo::gbtEfgSupportWithActiveInfo(const gbtEfgSupport& given)
  : gbtEfgSupport(given), 
    is_infoset_active(0,given.GetGame()->NumPlayers()), 
    is_nonterminal_node_active(0,given.GetGame()->NumPlayers())
{
  InitializeActiveLists();
}

gbtEfgSupportWithActiveInfo::gbtEfgSupportWithActiveInfo(
				  const gbtEfgSupportWithActiveInfo& given)
  : gbtEfgSupport(given.UnderlyingSupport()), 
    //is_infoset_active(0,given.GetGame()->NumPlayers()), 
        is_infoset_active(is_infoset_active), 
    is_nonterminal_node_active(given.is_nonterminal_node_active)
{
  //  InitializeActiveLists();
}

gbtEfgSupportWithActiveInfo::~gbtEfgSupportWithActiveInfo()
{}

// Operators
gbtEfgSupportWithActiveInfo &
gbtEfgSupportWithActiveInfo::operator=(const gbtEfgSupportWithActiveInfo &s)
{
  if (this != &s) {
    ((gbtEfgSupport&) *this) = s;
    is_infoset_active = s.is_infoset_active;
    is_nonterminal_node_active = s.is_nonterminal_node_active;
  }
  return *this;
}

bool 
gbtEfgSupportWithActiveInfo::operator==(const gbtEfgSupportWithActiveInfo &s) const
{
  if ((gbtEfgSupport&) *this != (gbtEfgSupport&) s) {
    //  gout << "Underlying supports differ.\n"; 
    return false; 
  }
  
  if (is_infoset_active != s.is_infoset_active) {
    //    gout<< "Active infosets differ:\n"; 
   
    //  for(int i = 0; i < is_infoset_active.Length(); i++)
    //  gout << "is_infoset_active[" << i << "] = " << is_infoset_active[i];

    //for(int i = 0; i < s.is_infoset_active.Length(); i++)
    //  gout << "s.is_infoset_active[" << i << "] = " << s.is_infoset_active[i];

    return false;
  }
  if (is_nonterminal_node_active != s.is_nonterminal_node_active) {
    // gout << "Active nodes differ.\n";
    return false;
  }
  return true;
}

bool 
gbtEfgSupportWithActiveInfo::operator!=(const gbtEfgSupportWithActiveInfo &s) const
{
  return !(*this == s);
}

gbtList<Gambit::GameNode> 
gbtEfgSupportWithActiveInfo::ReachableNodesInInfoset(const Gambit::GameInfoset &i) const
{
  gbtList<Gambit::GameNode> answer;
  int pl = i->GetPlayer()->GetNumber();
  int iset = i->GetNumber();
  for (int j = 1; j <= i->NumMembers(); j++)
    if (is_nonterminal_node_active[pl][iset][j])
      answer.Append(i->GetMember(j));
  return answer;
}

gbtList<Gambit::GameNode>
gbtEfgSupportWithActiveInfo::ReachableNonterminalNodes() const
{
  gbtList<Gambit::GameNode> answer;
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer p = GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= p->NumInfosets(); iset++)
      answer += ReachableNodesInInfoset(p->GetInfoset(iset));
  }
  return answer;
}

// Editing functions
void gbtEfgSupportWithActiveInfo::AddAction(const Gambit::GameAction &s)
{
  gbtEfgSupport::AddAction(s);

  gbtList<Gambit::GameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    activate_this_and_lower_nodes(startlist[i]);
}

bool gbtEfgSupportWithActiveInfo::RemoveAction(const Gambit::GameAction &s)
{
  gbtList<Gambit::GameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes(startlist[i]->GetChild(s->GetNumber()));

  // the following returns false if s was not active
  return gbtEfgSupport::RemoveAction(s);
}

bool 
gbtEfgSupportWithActiveInfo::RemoveActionReturningDeletedInfosets(const Gambit::GameAction &s,
					   gbtList<Gambit::GameInfoset> *list)
{

  gbtList<Gambit::GameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                           startlist[i]->GetChild(s->GetNumber()),list);

  // the following returns false if s was not active
  return gbtEfgSupport::RemoveAction(s);
}

int gbtEfgSupportWithActiveInfo::NumActiveNodes(const int pl,
					     const int iset) const
{
  int answer = 0;
  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    if (is_nonterminal_node_active[pl][iset][i])
      answer++;
  return answer;
}

int gbtEfgSupportWithActiveInfo::NumActiveNodes(const Gambit::GameInfoset &i) const
{
  return NumActiveNodes(i->GetPlayer()->GetNumber(),i->GetNumber());
}

bool gbtEfgSupportWithActiveInfo::InfosetIsActive(const int pl,
					      const int iset) const
{
  return is_infoset_active[pl][iset];
}

bool gbtEfgSupportWithActiveInfo::InfosetIsActive(const Gambit::GameInfoset &i) const
{
  return InfosetIsActive(i->GetPlayer()->GetNumber(),i->GetNumber());
}

bool gbtEfgSupportWithActiveInfo::NodeIsActive(const int pl,
					   const int iset,
					   const int node) const
{
  return is_nonterminal_node_active[pl][iset][node];
}

bool gbtEfgSupportWithActiveInfo::NodeIsActive(const Gambit::GameNode &n) const
{
  return NodeIsActive(n->GetInfoset()->GetPlayer()->GetNumber(),
		      n->GetInfoset()->GetNumber(),
		      n->NumberInInfoset());
}

bool gbtEfgSupportWithActiveInfo::HasActiveActionsAtActiveInfosets()
{
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame()->GetPlayer(pl)->NumInfosets(); iset++) 
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame()->GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
  return true;
}

bool gbtEfgSupportWithActiveInfo::HasActiveActionsAtActiveInfosetsAndNoOthers()
{
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame()->GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
      if (!InfosetIsActive(pl,iset))
        if ( NumActions(GetGame()->GetPlayer(pl)->GetInfoset(iset)) > 0 )
          return false;
      }
  return true;
}

