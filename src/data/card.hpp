//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/error.hpp>
#include <data/filter.hpp>
#include <data/field.hpp> // for Card::value

class Game;
class Dependency;
class Keyword;
DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Value);
DECLARE_POINTER_TYPE(StyleSheet);

// ----------------------------------------------------------------------------- : Card

/// A card from a card Set
class Card : public IntrusivePtrVirtualBase, public IntrusiveFromThis<Card> {
public:
  /// Default constructor, uses game_for_new_cards to make the game
  Card();
  /// Creates a card using the given game
  Card(const Game& game);
  
  /// The values on the fields of the card.
  /** The indices should correspond to the card_fields in the Game */
  IndexMap<FieldP, ValueP> data;
  /// Notes for this card
  String notes;
  /// Time the card was created/last modified
  wxDateTime time_created, time_modified;
  /// Alternative style to use for this card
  /** Optional; if not set use the card style from the set */
  StyleSheetP stylesheet;
  /// Alternative options to use for this card, for this card's stylesheet
  /** Optional; if not set use the styling data from the set.
   *  If stylesheet is set then contains data for the this->stylesheet, otherwise for set->stylesheet
   */
  IndexMap<FieldP,ValueP> styling_data;
  /// Is the styling_data set?
  bool has_styling;
  
  /// Extra values for specitic stylesheets, indexed by stylesheet name
  DelayedIndexMaps<FieldP,ValueP> extra_data;
  /// Styling information for a particular stylesheet
  IndexMap<FieldP, ValueP>& extraDataFor(const StyleSheet& stylesheet);
  
  /// Keyword usage statistics
  vector<pair<const Value*,const Keyword*>> keyword_usage;
  
  /// Get the identification of this card, an identification is something like a name, title, etc.
  /** May return "" */
  String identification() const;
  /// Does any field contains the given query string?
  bool contains(QuickFilterPart const& query) const;
  
  /// Find a value in the data by name and type
  template <typename T> T& value(const String& name) {
    for(IndexMap<FieldP, ValueP>::iterator it = data.begin() ; it != data.end() ; ++it) {
      if ((*it)->fieldP->name == name) {
        T* ret = dynamic_cast<T*>(it->get());
        if (!ret) throw InternalError(_("Card field with name '")+name+_("' doesn't have the right type"));
        return *ret;
      }
    }
    throw InternalError(_("Expected a card field with name '")+name+_("'"));
  }
  template <typename T> const T& value(const String& name) const {
    for(IndexMap<FieldP, ValueP>::const_iterator it = data.begin() ; it != data.end() ; ++it) {
      if ((*it)->fieldP->name == name) {
        const T* ret = dynamic_cast<const T*>(it->get());
        if (!ret) throw InternalError(_("Card field with name '")+name+_("' doesn't have the right type"));
        return *ret;
      }
    }
    throw InternalError(_("Expected a card field with name '")+name+_("'"));
  }
  
  DECLARE_REFLECTION();
};

inline String type_name(const Card&) {
  return _TYPE_("card");
}
inline String type_name(const vector<CardP>&) {
  return _TYPE_("cards"); // not actually used, only for locale.pl script
}

void mark_dependency_member(const Card& value, const String& name, const Dependency& dep);

