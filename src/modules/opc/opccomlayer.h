/*******************************************************************************
 * Copyright (c) 2012 -2015 AIT, fortiss GmbH
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *   Filip Andren, Alois Zoitl - initial API and implementation and/or initial documentation
 *******************************************************************************/
#ifndef OPCCOMLAYER_H_
#define OPCCOMLAYER_H_

#include <forte_config.h>
#include "comlayer.h"
#include "opcprocessvar.h"

class CIEC_ANY;
class COpcConnection;
struct Variant;

namespace forte {

  namespace com_infra {

    class COpcComLayer : public CComLayer{
      public:
        COpcComLayer(CComLayer* pa_poUpperLayer, CCommFB* pa_poComFB);
        virtual ~COpcComLayer();

        EComResponse sendData(void *pa_pvData, unsigned int pa_unSize); // top interface, called from top
        EComResponse recvData(const void *pa_pvData, unsigned int pa_unSize);
        EComResponse processInterrupt();

      protected:
        void closeConnection();

      private:
        EComResponse openConnection(char *pa_acLayerParameter);
        int addOpcItems();
        void setOutputValue(CIEC_ANY *pa_pDataOut, Variant * pa_pValue);
        void processClientParams(char* pa_acLayerParams);
        void convertInputData(void *pa_pData, unsigned int pa_nSize);

        unsigned int getInputValueSize(CIEC_ANY* pa_pData, Variant * pa_pNewValue);

        template<typename T>
        void getInputValue(void * pa_pData, Variant * pa_pNewValue);

        const char* m_acHost;
        const char* m_acServerName;
        long m_nUpdateRate;
        float m_nDeadBand;
        const char* m_acOpcGroupName;

        bool m_bLayerParamsOK;

        COpcConnection *m_pOpcConnection;

        typedef CSinglyLinkedList<COpcProcessVar*> TOpcProcessVarList;
        TOpcProcessVarList m_lFBOutputVars;
        TOpcProcessVarList m_lFBInputVars;

        EComResponse m_eInterruptResp;
        typedef CSinglyLinkedList<EComResponse> TComResponseList;
        TComResponseList m_lComResponses;

        CSyncObject m_oSync;
    };

  }

}

#endif /* OPCCOMLAYER_H_ */
