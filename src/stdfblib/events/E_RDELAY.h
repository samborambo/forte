/*******************************************************************************
 * Copyright (c) 2014 fortiss GmbH
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *   Alois Zoitl - initial API and implementation and/or initial documentation
 *******************************************************************************/
#ifndef _E_RDELAY_H_
#define _E_RDELAY_H_

#include "../timedfb.h"

/*! \brief Implementation of the E_RDELAY FB.
 */
class E_RDELAY : public CTimedFB{
  DECLARE_FIRMWARE_FB(E_RDELAY)
private:

  void executeEvent(int pa_nEIID);
public:
  E_RDELAY(const CStringDictionary::TStringId pa_nInstanceNameId, CResource *pa_poSrcRes);
	virtual ~E_RDELAY(){};

};

#endif /*E_RDELAY_H_*/
