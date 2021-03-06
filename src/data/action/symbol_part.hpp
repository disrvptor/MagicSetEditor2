//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file data/action/symbol_part.hpp
 *
 *  Actions operating on the insides of SymbolParts/SymbolShapes (ControlPoints and the like).
 */

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/action_stack.hpp>
#include <data/symbol.hpp>

// ----------------------------------------------------------------------------- : Utility

/// Constrain a vector to be horizontal, vertical or diagonal.
/** If constraint==false does nothing
 */
Vector2D constrain_vector(const Vector2D& v, bool constrain = true, bool only_diagonal = false);

/// Snap a vector to the grid with the given number of steps per unit interval.
/** If spacing==0 does not snap. */
Vector2D snap_vector(const Vector2D& v, int steps);

/// Add a delta to a vector
/** Possibly constrain the delta, and snap to result to grid
 */
Vector2D constrain_snap_vector(const Vector2D& v, const Vector2D& d, bool constrain, int steps);

/// Constrain a vector a vector, and snap to an offset grid
Vector2D constrain_snap_vector_offset(const Vector2D& off1, const Vector2D& d, bool constrain, int steps);
/// Constrain a vector a vector, and snap to two possible offset grids
/** Takes the closest snap */
Vector2D constrain_snap_vector_offset(const Vector2D& off1, const Vector2D& off2, const Vector2D& d, bool constrain, int steps);

// ----------------------------------------------------------------------------- : Base class

/// An action that by itself doesn't do anything, but can later be extended after it is performed
class ExtendableAction : public Action {
protected:
  bool done = false;
};

// ----------------------------------------------------------------------------- : Move control point

/// Moving a control point in a symbol
class ControlPointMoveAction : public ExtendableAction {
public:
  ControlPointMoveAction(const set<ControlPointP>& points);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
  
  /// Update this action to move some more
  void move(const Vector2D& delta);
  
private:
  set<ControlPointP> points;  ///< Points to move
  vector<Vector2D> oldValues; ///< Their old positions
  Vector2D delta;        ///< Amount we moved
public:
  bool constrain;        ///< Constrain movement?
  int snap;          ///< Snap to grid?
};

// ----------------------------------------------------------------------------- : Move handle

/// Moving a handle(before/after) of a control point in a symbol
class HandleMoveAction : public ExtendableAction {
public:
  HandleMoveAction(const SelectedHandle& handle);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
  
  /// Update this action to move some more
  void move(const Vector2D& delta);
  
private:
  SelectedHandle handle;    ///< The handle to move
  Vector2D old_handle;    ///< Old value of this handle
  Vector2D old_other;      ///< Old value of other handle, needed for contraints
  Vector2D delta;        ///< Amount we moved
public:
  bool constrain;        ///< Constrain movement?
  int snap;          ///< Snap to grid?
};

// ----------------------------------------------------------------------------- : Segment mode

/// Utility class to update a control point
class ControlPointUpdate {
public:
  ControlPointUpdate(const ControlPointP& pnt);
  
  /// Perform or undo an update on this control point
  void perform();
  
  /// Other value that is swapped with the current one.
  /// Should be changed to make perform have an effect
  ControlPoint other;
  /// The point that is to be changed, should not be updated before perform()
  ControlPointP point;
};


/// Changing a line to a curve and vice versa
class SegmentModeAction : public Action {
public:
  SegmentModeAction(const ControlPointP& p1, const ControlPointP& p2, SegmentMode mode);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
  
protected:
  ControlPointUpdate point1, point2;
};

// ----------------------------------------------------------------------------- : Locking mode

/// Locking a control point
class LockModeAction : public Action {
public:
  LockModeAction(const ControlPointP& p, LockMode mode);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
  
private:
  ControlPointUpdate point;  ///< The affected point
};

// ----------------------------------------------------------------------------- : Move curve

/// Dragging a curve; also coverts lines to curves
/** Inherits from SegmentModeAction because it also has that effect
 */
class CurveDragAction : public SegmentModeAction {
public:
  CurveDragAction(const ControlPointP& point1, const ControlPointP& point2);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
  
  // Move the curve by this much, it is grabbed at time t
  void move(const Vector2D& delta, double t);
};

// ----------------------------------------------------------------------------- : Add control point

/// Insert a new point in a symbol shape
class ControlPointAddAction : public Action {
public:
  /// Insert a new point in shape, after position insertAfter_, at the time t on the segment
  ControlPointAddAction(const SymbolShapeP& shape, UInt insert_after, double t);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
  
  inline ControlPointP getNewPoint() const { return new_point; }
  
private:
  SymbolShapeP  shape;        ///< SymbolShape we are in
  ControlPointP new_point;      ///< The point to insert
  UInt          insert_after;      ///< Insert after index .. in the array
  ControlPointUpdate point1, point2;  ///< Update the points around the new point
};

// ----------------------------------------------------------------------------- : Remove control point

/// Action that removes any number of points from a symbol shape
/// TODO: If less then 3 points are left removes the entire shape?
unique_ptr<Action> control_point_remove_action(const SymbolShapeP& shape, const set<ControlPointP>& to_delete);




// ----------------------------------------------------------------------------- : Move symmetry center/handle

/// Moving the handle or the center of a symbol symmetry
class SymmetryMoveAction : public Action {
public:
  SymmetryMoveAction(SymbolSymmetry& symmetry, bool is_handle);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
  
  /// Update this action to move some more
  void move(const Vector2D& delta);
  
private:
  SymbolSymmetry& symmetry; ///< Affected part
  bool is_handle;      ///< Move the handle or the center?
  Vector2D delta;      ///< Amount we moved
  Vector2D original;    ///< Original value
public:
  bool constrain;      ///< Constrain movement?
  int snap;        ///< Snap to grid?
};

// ----------------------------------------------------------------------------- : Change symmetry kind

/// Change the type of symmetry
class SymmetryTypeAction : public Action {
public:
  SymmetryTypeAction(SymbolSymmetry& symmetry, SymbolSymmetryType type);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
private:
  SymbolSymmetry&    symmetry;
  SymbolSymmetryType type;
  String             old_name;
  int                copies; /// may be changed to make it a multiple of two
};

// ----------------------------------------------------------------------------- : Change symmetry copies

/// Change the number of copies of a symmetry
class SymmetryCopiesAction : public Action {
public:
  SymmetryCopiesAction(SymbolSymmetry& symmetry, int copies);
  
  String getName(bool to_undo) const override;
  void perform(bool to_undo) override;
private:
  SymbolSymmetry& symmetry;
  int             copies;
  String          old_name;
};

