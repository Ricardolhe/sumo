/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2022 German Aerospace Center (DLR) and others.
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
/// @file    MFXToggleButtonTooltip.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Sep 2022
///
// Button similar to FXToggleButton but with the possibility of showing tooltips
/****************************************************************************/
#include <config.h>

#include "MFXToggleButtonTooltip.h"


FXDEFMAP(MFXToggleButtonTooltip) MFXToggleButtonTooltipMap[] = {
    FXMAPFUNC(SEL_ENTER,    0,  MFXToggleButtonTooltip::onEnter),
    FXMAPFUNC(SEL_LEAVE,    0,  MFXToggleButtonTooltip::onLeave),
};


// Object implementation
FXIMPLEMENT(MFXToggleButtonTooltip, FXToggleButton, MFXToggleButtonTooltipMap, ARRAYNUMBER(MFXToggleButtonTooltipMap))

MFXToggleButtonTooltip::MFXToggleButtonTooltip(FXComposite* p, MFXStaticToolTip* staticToolTip, 
        const FXString& text1, const FXString& text2, FXIcon* ic1, FXIcon* ic2, FXObject* tgt, 
        FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb) :
    FXToggleButton(p, text1, text2, ic1, ic2, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb),
    myStaticToolTip(staticToolTip) {
}


MFXToggleButtonTooltip::~MFXToggleButtonTooltip() {}


long
MFXToggleButtonTooltip::onEnter(FXObject* sender, FXSelector sel, void* ptr) {
    // show tip show
    myStaticToolTip->onTipShow(sender, sel, ptr);
    return FXToggleButton::onEnter(sender, sel, ptr);
}


long
MFXToggleButtonTooltip::onLeave(FXObject* sender, FXSelector sel, void* ptr) {
    // hide tip show
    myStaticToolTip->onTipHide(sender, sel, this);
    return FXToggleButton::onLeave(sender, sel, ptr);
}


/****************************************************************************/
