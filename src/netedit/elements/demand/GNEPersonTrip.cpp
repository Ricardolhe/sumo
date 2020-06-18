/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GNEPersonTrip.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// A class for visualizing person trips in Netedit
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEPersonTrip.h"
#include "GNERouteHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge,
        double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(net->generateDemandElementID(GNE_TAG_PERSONTRIP_EDGE_EDGE), net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_EDGE_EDGE,
        {}, {fromEdge, toEdge}, {}, {}, {}, {}, {personParent}, {}, // Parents
        {}, {}, {}, {}, {}, {}, {}, {}),                            // Childrens
    myArrivalPosition(arrivalPosition),
    myVTypes(types),
    myModes(modes) {
    // compute person trip
    computePath();
}


GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEAdditional* toBusStop,
    double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(net->generateDemandElementID(GNE_TAG_PERSONTRIP_EDGE_BUSSTOP), net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_EDGE_BUSSTOP,
        {}, {fromEdge}, {}, {toBusStop}, {}, {}, {personParent}, {},    // Parents
        {}, {}, {}, {}, {}, {}, {}, {}),                                // Childrens
    myArrivalPosition(arrivalPosition),
    myVTypes(types),
    myModes(modes) {
    // compute person trip
    computePath();
}

GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEEdge* toEdge,
    double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(net->generateDemandElementID(GNE_TAG_PERSONTRIP_BUSSTOP_EDGE), net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_BUSSTOP_EDGE,
        {}, {toEdge}, {}, {fromBusStop}, {}, {}, {personParent}, {},    // Parents
        {}, {}, {}, {}, {}, {}, {}, {}),                                // Childrens
    myArrivalPosition(arrivalPosition),
    myVTypes(types),
    myModes(modes) {
    // compute person trip
    computePath();
}


GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEAdditional* toBusStop,
    double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(net->generateDemandElementID(GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP), net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP,
        {}, {}, {}, {fromBusStop, toBusStop}, {}, {}, {personParent}, {},   // Parents
        {}, {}, {}, {}, {}, {}, {}, {}),                                    // Childrens
    myArrivalPosition(arrivalPosition),
    myVTypes(types),
    myModes(modes) {
    // compute person trip
    computePath();
}


GNEPersonTrip::~GNEPersonTrip() {}


GUIGLObjectPopupMenu*
GNEPersonTrip::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    // build header
    buildPopupHeader(ret, app);
    // build menu command for center button and copy cursor position to clipboard
    buildCenterPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    // buld menu commands for names
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " name to clipboard").c_str(), nullptr, ret, MID_COPY_NAME);
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " typed name to clipboard").c_str(), nullptr, ret, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(ret);
    // build selection and show parameters menu
    myNet->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    // show option to open demand element dialog
    if (myTagProperty.hasDialog()) {
        new FXMenuCommand(ret, ("Open " + getTagStr() + " Dialog").c_str(), getIcon(), &parent, MID_OPEN_ADDITIONAL_DIALOG);
        new FXMenuSeparator(ret);
    }
    new FXMenuCommand(ret, ("Cursor position in view: " + toString(getPositionInView().x()) + "," + toString(getPositionInView().y())).c_str(), nullptr, nullptr, 0);
    return ret;
}


void
GNEPersonTrip::writeDemandElement(OutputDevice& device) const {
    // open tag
    device.openTag(SUMO_TAG_PERSONTRIP);
    // check if we have to write "from" attributes
    if (getParentDemandElements().at(0)->getPreviousChildDemandElement(this) == nullptr) {
        // write "to" attributes depending of start and end
        if (myTagProperty.personPlanStartEdge()) {
            device.writeAttr(SUMO_ATTR_FROM, getParentEdges().front()->getID());
        } else if (myTagProperty.personPlanStartBusStop()) {
            device.writeAttr(SUMO_ATTR_FROM, getParentAdditionals().front()->getID());
        }
    }
    // write "to" attributes depending of start and end
    if (myTagProperty.personPlanStartEdge()) {
        device.writeAttr(SUMO_ATTR_TO, getParentEdges().back()->getID());
    } else if (myTagProperty.personPlanStartBusStop()) {
        device.writeAttr(SUMO_ATTR_BUS_STOP, getParentAdditionals().back()->getID());
    }
    // write modes
    if (myModes.size() > 0) {
        device.writeAttr(SUMO_ATTR_MODES, myModes);
    }
    // write vTypes
    if (myVTypes.size() > 0) {
        device.writeAttr(SUMO_ATTR_VTYPES, myVTypes);
    }
    // only write arrivalPos if is different of -1
    if (myArrivalPosition != -1) {
        device.writeAttr(SUMO_ATTR_ARRIVALPOS, myArrivalPosition);
    }
    // write parameters
    writeParams(device);
    // close tag
    device.closeTag();
}


bool
GNEPersonTrip::isDemandElementValid() const {
    if ((getParentEdges().size() == 2) && (getParentEdges().at(0) == getParentEdges().at(1))) {
        // from and to are the same edges
        return true;
    } else if (getPath().size() > 0) {
        // if path edges isn't empty, then there is a valid route
        return true;
    } else {
        return false;
    }
}


std::string
GNEPersonTrip::getDemandElementProblem() const {
    if (getParentEdges().size() == 0) {
        return ("A person trip need at least one edge");
    } else {
        // check if exist at least a connection between every edge
        for (int i = 1; i < (int)getParentEdges().size(); i++) {
            if (myNet->getPathCalculator()->consecutiveEdgesConnected(getParentDemandElements().front()->getVClass(), getParentEdges().at((int)i - 1), getParentEdges().at(i)) == false) {
                return ("Edge '" + getParentEdges().at((int)i - 1)->getID() + "' and edge '" + getParentEdges().at(i)->getID() + "' aren't consecutives");
            }
        }
        // there is connections bewteen all edges, then all ok
        return "";
    }
}


void
GNEPersonTrip::fixDemandElementProblem() {
    // currently the only solution is removing PersonTrip
}


GNEEdge*
GNEPersonTrip::getFromEdge() const {
    if (getParentDemandElements().size() == 2) {
        // obtain position and rotation of first edge route
        return getParentDemandElements().at(1)->getFromEdge();
    } else {
        return getParentEdges().front();
    }
}


GNEEdge*
GNEPersonTrip::getToEdge() const {
    if (getParentDemandElements().size() == 2) {
        // obtain position and rotation of first edge route
        return getParentDemandElements().at(1)->getToEdge();
    } else {
        return getParentEdges().back();
    }
}


SUMOVehicleClass
GNEPersonTrip::getVClass() const {
    return getParentDemandElements().front()->getVClass();
}


const RGBColor&
GNEPersonTrip::getColor() const {
    return getParentDemandElements().front()->getColor();
}


void
GNEPersonTrip::startGeometryMoving() {
    // only start geometry moving if arrival position isn't -1
    if (myArrivalPosition != -1) {
        // always save original position over view
        myPersonTripMove.originalViewPosition = getPositionInView();
        // save arrival position
        myPersonTripMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_ARRIVALPOS);
        // save current centering boundary
        myPersonTripMove.movingGeometryBoundary = getCenteringBoundary();
    }
}


void
GNEPersonTrip::endGeometryMoving() {
    // check that myArrivalPosition isn't -1 and endGeometryMoving was called only once
    if ((myArrivalPosition != -1) && myPersonTripMove.movingGeometryBoundary.isInitialised()) {
        // reset myMovingGeometryBoundary
        myPersonTripMove.movingGeometryBoundary.reset();
    }
}


void
GNEPersonTrip::moveGeometry(const Position& offset) {
    // only move if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        // Calculate new position using old position
        Position newPosition = myPersonTripMove.originalViewPosition;
        newPosition.add(offset);
        // filtern position using snap to active grid
        newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
        // obtain lane shape (to improve code legibility)
        const PositionVector& laneShape = getParentEdges().back()->getLanes().front()->getLaneShape();
        // calculate offset lane
        double offsetLane = laneShape.nearest_offset_to_point2D(newPosition, false) - laneShape.nearest_offset_to_point2D(myPersonTripMove.originalViewPosition, false);
        // Update arrival Position
        myArrivalPosition = parse<double>(myPersonTripMove.firstOriginalLanePosition) + offsetLane;
        // Update geometry
        updateGeometry();
    }
}


void
GNEPersonTrip::commitGeometryMoving(GNEUndoList* undoList) {
    // only commit geometry moving if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        undoList->p_begin("arrivalPos of " + getTagStr());
        undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_ARRIVALPOS, toString(myArrivalPosition), myPersonTripMove.firstOriginalLanePosition));
        undoList->p_end();
    }
}


void
GNEPersonTrip::updateGeometry() {
    // calculate person plan start and end positions
    GNEGeometry::ExtremeGeometry extremeGeometry = calculatePersonPlanLaneStartEndPos();
    // calculate edge geometry path using path
    GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getPath(), extremeGeometry);
    // update child demand elementss
    for (const auto& i : getChildDemandElements()) {
        i->updateGeometry();
    }
}


void
GNEPersonTrip::updateDottedContour() {
    //
}


void
GNEPersonTrip::updatePartialGeometry(const GNELane* lane) {
    // calculate person plan start and end positions
    GNEGeometry::ExtremeGeometry extremeGeometry = calculatePersonPlanLaneStartEndPos();
    // calculate geometry path
    GNEGeometry::updateGeometricPath(myDemandElementSegmentGeometry, lane, extremeGeometry);
    // update child demand elementss
    for (const auto& i : getChildDemandElements()) {
        i->updatePartialGeometry(lane);
    }
}


void
GNEPersonTrip::computePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_EDGE) {
        calculatePathLanes(getVClass(), true, 
            getFirstAllowedVehicleLane(), 
            getLastAllowedVehicleLane(), 
            {});
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_BUSSTOP) {
        calculatePathLanes(getVClass(), true, 
            getFirstAllowedVehicleLane(), 
            getParentAdditionals().back()->getParentLanes().front(), 
            {});
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_EDGE) {
        calculatePathLanes(getVClass(), true, 
            getParentAdditionals().front()->getParentLanes().front(), 
            getLastAllowedVehicleLane(),
            {});
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP) {
        calculatePathLanes(getVClass(), true, 
            getParentAdditionals().front()->getParentLanes().front(), 
            getParentAdditionals().back()->getParentLanes().front(), 
            {});
    }
    // update geometry
    updateGeometry();
}


void
GNEPersonTrip::invalidatePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_EDGE) {
        resetPathLanes(getVClass(), true, 
            getFirstAllowedVehicleLane(), 
            getLastAllowedVehicleLane(), 
            {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_BUSSTOP) {
        resetPathLanes(getVClass(), true, 
            getFirstAllowedVehicleLane(), 
            getParentAdditionals().back()->getParentLanes().front(), 
            {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_EDGE) {
        resetPathLanes(getVClass(), true, 
            getParentAdditionals().front()->getParentLanes().front(), 
            getLastAllowedVehicleLane(),
            {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_BUSSTOP) {
        resetPathLanes(getVClass(), true, 
            getParentAdditionals().front()->getParentLanes().front(), 
            getParentAdditionals().back()->getParentLanes().front(), 
            {});
    }
    // update geometry
    updateGeometry();
}


Position
GNEPersonTrip::getPositionInView() const {
    return Position();
}


std::string
GNEPersonTrip::getParentName() const {
    return getParentDemandElements().front()->getID();
}


Boundary
GNEPersonTrip::getCenteringBoundary() const {
    Boundary personTripBoundary;
    // return the combination of all parent edges's boundaries
    for (const auto& i : getParentEdges()) {
        personTripBoundary.add(i->getCenteringBoundary());
    }
    // check if is valid
    if (personTripBoundary.isInitialised()) {
        return personTripBoundary;
    } else {
        return Boundary(-0.1, -0.1, 0.1, 0.1);
    }
}


void
GNEPersonTrip::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEPersonTrip::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // PersonTrips are drawn in GNEEdges
}


void 
GNEPersonTrip::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane) const {
    // get GNEViewNet
    GNEViewNet* viewNet = lane->getNet()->getViewNet();
    // declare flag to enable or disable draw person plan
    bool drawPersonPlan = false;
    if (viewNet->getDemandViewOptions().showAllPersonPlans()) {
        drawPersonPlan = true;
    } else if (viewNet->getDottedAC() == getParentDemandElements().front()) {
        drawPersonPlan = true;
    } else if (viewNet->getDemandViewOptions().getLockedPerson() == getParentDemandElements().front()) {
        drawPersonPlan = true;
    } else if (viewNet->getDottedAC() && viewNet->getDottedAC()->getTagProperty().isPersonPlan() &&
        (viewNet->getDottedAC()->getAttribute(GNE_ATTR_PARENT) == getAttribute(GNE_ATTR_PARENT))) {
        drawPersonPlan = true;
    }
    // check if draw person plan elements can be drawn
    if (drawPersonPlan) {
        // calculate myDemandElement width
        double myDemandElementWidth = 0;
        // flag to check if width must be duplicated
        bool duplicateWidth = (viewNet->getDottedAC() == this) || (viewNet->getDottedAC() == getParentDemandElements().front()) ? true : false;
        // Set width depending of person plan type
        if (myTagProperty.isPersonTrip()) {
            myDemandElementWidth = s.addSize.getExaggeration(s, lane) * s.widthSettings.personTrip;
        } else if (myTagProperty.isWalk()) {
            myDemandElementWidth = s.addSize.getExaggeration(s, lane) * s.widthSettings.walk;
        } else if (myTagProperty.isRide()) {
            myDemandElementWidth = s.addSize.getExaggeration(s, lane) * s.widthSettings.ride;
        }
        // check if width has to be duplicated
        if (duplicateWidth) {
            myDemandElementWidth *= 2;
        }
        // set myDemandElement color
        RGBColor myDemandElementColor;
        // Set color depending of person plan type
        if (drawUsingSelectColor()) {
            myDemandElementColor = s.colorSettings.selectedPersonPlanColor;
        } else if (myTagProperty.isPersonTrip()) {
            myDemandElementColor = s.colorSettings.personTrip;
        } else if (myTagProperty.isWalk()) {
            myDemandElementColor = s.colorSettings.walk;
        } else if (myTagProperty.isRide()) {
            myDemandElementColor = s.colorSettings.ride;
        }
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, getType());
        // iterate over segments
        for (const auto& segment : myDemandElementSegmentGeometry) {
            // draw partial segment
            if (segment.getLane() == lane) {
                // Set person plan color (needed due drawShapeDottedContour)
                GLHelper::setColor(myDemandElementColor);
                // draw box line
                GNEGeometry::drawSegmentGeometry(viewNet, segment, myDemandElementWidth);
                // check if shape dotted contour has to be drawn
                if (viewNet->getDottedAC() == this) {
                    GNEGeometry::drawSegmentGeometry(viewNet, segment, myDemandElementWidth);
                }
            }
        }
        // Pop last matrix
        glPopMatrix();

        // Draw name if isn't being drawn for selecting
        if (!s.drawForRectangleSelection) {
            drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
        }
        // Pop name
        glPopName();
        // check if person plan ArrivalPos attribute
        if (myTagProperty.hasAttribute(SUMO_ATTR_ARRIVALPOS)) {
            // obtain arrival position using last segment
            const Position& arrivalPos = getDemandElementSegmentGeometry().getLastPosition();
            // only draw arrival position point if isn't -1
            if (arrivalPos != Position::INVALID) {
                // obtain circle width
                const double circleWidth = (duplicateWidth ? SNAP_RADIUS : (SNAP_RADIUS / 2.0)) * MIN2((double)0.5, s.laneWidthExaggeration);
                const double circleWidthSquared = circleWidth * circleWidth;
                if (!s.drawForRectangleSelection || (viewNet->getPositionInformation().distanceSquaredTo2D(arrivalPos) <= (circleWidthSquared + 2))) {
                    glPushMatrix();
                    // translate to pos and move to upper using GLO_PERSONTRIP (to avoid overlapping)
                    glTranslated(arrivalPos.x(), arrivalPos.y(), GLO_PERSONTRIP + 0.01);
                    // Set color depending of person plan type
                    if (drawUsingSelectColor()) {
                        GLHelper::setColor(s.colorSettings.selectedPersonPlanColor);
                    } else if (myTagProperty.isPersonTrip()) {
                        GLHelper::setColor(s.colorSettings.personTrip);
                    } else if (myTagProperty.isWalk()) {
                        GLHelper::setColor(s.colorSettings.walk);
                    } else if (myTagProperty.isRide()) {
                        GLHelper::setColor(s.colorSettings.ride);
                    }
                    // resolution of drawn circle depending of the zoom (To improve smothness)
                    GLHelper::drawFilledCircle(circleWidth, s.getCircleResolution());
                    glPopMatrix();
                }
            }
        }
    }
    // draw person if this edge correspond to the first edge of first Person's person plan
    const GNEDemandElement* firstPersonPlan = getParentDemandElements().front()->getChildDemandElements().front();
    const GNEEdge* firstEdge = GNERouteHandler::getFirstPersonPlanEdge(firstPersonPlan);
    // draw person parent if this is the edge first edge and this is the first plan
    if ((firstEdge == lane->getParentEdge()) && (firstPersonPlan == this)) {
        getParentDemandElements().front()->drawGL(s);
    }
}


void 
GNEPersonTrip::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane) const {
    // get GNEViewNet
    GNEViewNet* viewNet = myNet->getViewNet();
    // declare flag to enable or disable draw person plan
    bool drawPersonPlan = false;
    if (viewNet->getDemandViewOptions().showAllPersonPlans()) {
        drawPersonPlan = true;
    } else if (viewNet->getDottedAC() == getParentDemandElements().front()) {
        drawPersonPlan = true;
    } else if (viewNet->getDemandViewOptions().getLockedPerson() == getParentDemandElements().front()) {
        drawPersonPlan = true;
    } else if (viewNet->getDottedAC() && viewNet->getDottedAC()->getTagProperty().isPersonPlan() &&
        (viewNet->getDottedAC()->getAttribute(GNE_ATTR_PARENT) == getAttribute(GNE_ATTR_PARENT))) {
        drawPersonPlan = true;
    }
    // check if draw person plan elements can be drawn
    if (drawPersonPlan) {
        // obtain lane2lane geometry
        const GNEGeometry::Geometry &lane2laneGeometry = fromLane->getLane2laneConnections().connectionsMap.at(toLane);
        // flag to check if width must be duplicated
        bool duplicateWidth = (myNet->getViewNet()->getDottedAC() == this) || (myNet->getViewNet()->getDottedAC() == getParentDemandElements().front()) ? true : false;
        // get width
        double width = s.addSize.getExaggeration(s, fromLane) * s.widthSettings.personTrip;
        // check if width has to be duplicated
        if (duplicateWidth) {
            width *= 2;
        }
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, getType());
        // Set color of the base
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedVehicleColor);
        } else {
            GLHelper::setColor(s.colorSettings.personTrip);
        }
        // draw lane2lane
        GNEGeometry::drawGeometry(myNet->getViewNet(), lane2laneGeometry, width);
        // Pop last matrix
        glPopMatrix();
    }
}


std::string
GNEPersonTrip::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_FROM:
            return getParentEdges().front()->getID();
        case SUMO_ATTR_TO:
            return getParentEdges().back()->getID();
        case GNE_ATTR_FROM_BUSSTOP:
            return getParentAdditionals().front()->getID();
        case GNE_ATTR_TO_BUSSTOP:
            return getParentAdditionals().back()->getID();
        // specific person plan attributes
        case SUMO_ATTR_MODES:
            return joinToString(myModes, " ");
        case SUMO_ATTR_VTYPES:
            return joinToString(myVTypes, " ");
        case SUMO_ATTR_ARRIVALPOS:
            if (myArrivalPosition == -1) {
                return "";
            } else {
                return toString(myArrivalPosition);
            }
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        case GNE_ATTR_PARENT:
            return getParentDemandElements().front()->getID();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEPersonTrip::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ARRIVALPOS:
            if (myArrivalPosition != -1) {
                return myArrivalPosition;
            } else {
                return (getLastAllowedVehicleLane()->getLaneShape().length() - POSITION_EPS);
            }
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPersonTrip::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_TO_BUSSTOP:
        // specific person plan attributes
        case SUMO_ATTR_MODES:
        case SUMO_ATTR_VTYPES:
        case SUMO_ATTR_ARRIVALPOS:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEPersonTrip::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveEdge(value, false) != nullptr);
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_TO_BUSSTOP:
            return (myNet->retrieveAdditional(SUMO_TAG_BUS_STOP, value, false) != nullptr);
        // specific person plan attributes
        case SUMO_ATTR_MODES: {
            SVCPermissions dummyModeSet;
            std::string dummyError;
            return SUMOVehicleParameter::parsePersonModes(value, myTagProperty.getTagStr(), getID(), dummyModeSet, dummyError);
        }
        case SUMO_ATTR_VTYPES:
            return canParse<std::vector<std::string> >(value);
        case SUMO_ATTR_ARRIVALPOS:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                const double parsedValue = canParse<double>(value);
                if ((parsedValue < 0) || (parsedValue > getLastAllowedVehicleLane()->getLaneShape().length())) {
                    return false;
                } else {
                    return true;
                }
            } else {
                return false;
            }
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPersonTrip::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNEPersonTrip::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


bool
GNEPersonTrip::isAttributeEnabled(SumoXMLAttr /*key*/) const {
    return true;
}


std::string
GNEPersonTrip::getPopUpID() const {
    return getTagStr();
}


std::string
GNEPersonTrip::getHierarchyName() const {
    if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_EDGE) {
        return "personTrip: " + getParentEdges().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_BUSSTOP) {
        return "personTrip: " + getParentEdges().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_EDGE) {
        return "personTrip: " + getParentAdditionals().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP) {
        return "personTrip: " + getParentAdditionals().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else {
        throw ("Invalid personTrip tag");
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNEPersonTrip::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
            // change first edge
            replaceFirstParentEdge(this, myNet->retrieveEdge(value));
            // compute person trip
            computePath();
            break;
        case SUMO_ATTR_TO:
            // change last edge
            replaceLastParentEdge(this, myNet->retrieveEdge(value));
            // compute person trip
            computePath();
            break;
        case GNE_ATTR_FROM_BUSSTOP:
            replaceParentAdditional(this, value, 0);
            // compute person trip
            computePath();
            break;
        case GNE_ATTR_TO_BUSSTOP:
            // -> check this
            if (getParentAdditionals().size() > 1) {
                replaceParentAdditional(this, value, 1);
            } else {
                replaceParentAdditional(this, value, 0);
            }
            // compute person trip
            computePath();
            break;
        // specific person plan attributes
        case SUMO_ATTR_MODES:
            myModes = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
            break;
        case SUMO_ATTR_VTYPES:
            myVTypes = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
            break;
        case SUMO_ATTR_ARRIVALPOS:
            if (value.empty()) {
                myArrivalPosition = -1;
            } else {
                myArrivalPosition = parse<double>(value);
            }
            updateGeometry();
            break;
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPersonTrip::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
