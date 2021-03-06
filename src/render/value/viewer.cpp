//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : ValueViewer

ValueViewer::ValueViewer(DataViewer& parent, const StyleP& style)
  : StyleListener(style), parent(parent)
  , bounding_box(style->getExternalRect())
{}

Package& ValueViewer::getStylePackage() const { return parent.getStylePackage(); }
Package& ValueViewer::getLocalPackage() const { return parent.getLocalPackage(); }

void ValueViewer::setValue(const ValueP& value) {
  assert(value->fieldP == styleP->fieldP); // matching field
  if (valueP == value) return;
  valueP = value;
  onValueChange();
}

bool ValueViewer::containsPoint(const RealPoint& p) const {
  return getMask().isOpaque(p, bounding_box.size());
}
RealRect ValueViewer::boundingBoxBorder() const {
  return bounding_box.grow(1);
}
bool ValueViewer::isVisible() const {
  return getStyle()->visible
    && bounding_box.width > 0
    && bounding_box.height > 0
    && fabs(bounding_box.x) < 100000
    && fabs(bounding_box.y) < 100000;
}

Rotation ValueViewer::getRotation() const {
  return Rotation(deg_to_rad(getStyle()->angle), bounding_box, 1.0, getStretch());
}

#if defined(__WXMSW__)
  // on windows, wxDOT is not actually dotted, so use a custom style to achieve that
  static wxDash dashes_dotted[] = { 0,2 };
  wxPen dotted_pen(wxColour const& color) {
    wxPen pen(color, 1, wxPENSTYLE_USER_DASH);
    pen.SetDashes(2, dashes_dotted);
    return pen;
  }
#else
  wxPen dotted_pen(wxColour const& color) {
    return wxPen(color, 1, wxPENSTYLE_DOT);
  }
#endif

bool ValueViewer::setFieldBorderPen(RotatedDC& dc) {
  if (!getField()->editable) return false;
  DrawWhat what = drawWhat();
  if (!(what & DRAW_BORDERS)) return false;
  if (what & DRAW_ACTIVE) {
    dc.SetPen(wxPen(Color(0, 128, 255), 1, wxPENSTYLE_SOLID));
  } else if (what & DRAW_HOVER) {
    dc.SetPen(dotted_pen(Color(0, 128, 255)));
  } else {
    dc.SetPen(dotted_pen(Color(128, 128, 128)));
  }
  return true;
}

void ValueViewer::drawFieldBorder(RotatedDC& dc) {
  if (setFieldBorderPen(dc)) {
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    const AlphaMask& alpha_mask = getMask(dc);
    if (alpha_mask.isLoaded()) {
      // from mask
      vector<wxPoint> points;
      alpha_mask.convexHull(points);
      if (points.size() < 3) return;
      FOR_EACH(p, points) p = dc.trPixelNoZoom(RealPoint(p.x,p.y));
      dc.getDC().DrawPolygon((int)points.size(), &points[0]);
    } else {
      // simple rectangle
      dc.DrawRectangle(dc.getInternalRect().grow(dc.trInvS(1)));
    }
  }
}

const AlphaMask& ValueViewer::getMask(int w, int h) const {
  GeneratedImage::Options opts(w, h, &getStylePackage(), &getLocalPackage());
  return styleP->mask.get(opts);
}
const AlphaMask& ValueViewer::getMask(const Rotation& rot) const {
  return getMask((int)rot.trX(styleP->width), (int)rot.trY(styleP->height));
}

Context& ValueViewer::getContext() const {
  return parent.getContext();
}

void ValueViewer::redraw() {
  parent.redraw(*this);
}

bool ValueViewer::nativeLook() const {
  return parent.nativeLook();
}
DrawWhat ValueViewer::drawWhat() const {
  return parent.drawWhat(this);
}
bool ValueViewer::isCurrent() const {
  return parent.viewerIsCurrent(this);
}

void ValueViewer::onStyleChange(int changes) {
  if (!(changes & CHANGE_ALREADY_PREPARED)) {
    parent.redraw(*this);
  }
  // update bounding box
  if (!nativeLook()) bounding_box = getStyle()->getExternalRect();
}
