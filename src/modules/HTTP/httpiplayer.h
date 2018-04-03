/*******************************************************************************
  * Copyright (c) 2010-2018 fortiss, TU Wien ACIN and others.
  * All rights reserved. This program and the accompanying materials
  * are made available under the terms of the Eclipse Public License v1.0
  * which accompanies this distribution, and is available at
  * http://www.eclipse.org/legal/epl-v10.html
  *
  * Contributors:
  *    Alois Zoitl - initial implementation, rework communication infrastructure and bug fixes
  *    Michael Hofmann - layer based communication infrastructure
  *    Marc Jakobi - created CHttpIPComLayer based on CIPComLayer with response receival optimizations due to HTTP servers
  *                  closing socket connections after a certain time.
 *******************************************************************************
 ******************************************************************************
 *
 *
 ********************************************************************************
 * IP Com Layer used by the HTTP Com Layer.
 * This class is similar to the CIPComLayer, with some changes for performance improvement.
 * Opening the connection, sending and receiving data all occurs in the sendData() method.
 * This is done because mose HTTP servers have high requirements for speed that currently cannot be
 * met by FORTE's standard CIPComLayer.
 * TODO (Suggestion by Marc Jakobi): Create common superclass for CIPComLayer and CHttpIPComLayer to eliminate code duplication.
********************************************************************************/

#ifndef _HTTPIPCOMLAYER_H_
#define _HTTPIPCOMLAYER_H_

#include "ipcomlayer.h"
#include "httplayer.h"
#include <time.h>

namespace forte {

  namespace com_infra {

    class CHttpIPComLayer : public CComLayer {
      public:
    CHttpIPComLayer(CComLayer* paUpperLayer, CBaseCommFB* paComFB);
        virtual ~CHttpIPComLayer();

    EComResponse sendData(void *paData, unsigned int paSize); // top interface, called from top
    EComResponse recvData(const void *paData, unsigned int paSize);

    EComResponse processInterrupt();

  protected:
    void closeConnection();

  private:
    void closeSocket(CIPComSocketHandler::TSocketDescriptor *paSocketID);

    /** Connection time out in s */
    const time_t kTimeOutS;

    EComResponse openConnection(char *paLayerParameter);
    EComResponse openHTTPConnection();
    void handledConnectedDataRecv();

    CIPComSocketHandler::TSocketDescriptor mSocketID;
    // CIPComSocketHandler::TSocketDescriptor m_nListeningID; //!> to be used by server type connections. there the m_nSocketID will be used for the accepted connection.
    CIPComSocketHandler::TUDPDestAddr mDestAddr;
    EComResponse mInterruptResp;
    char mRecvBuffer[cg_unIPLayerRecvBufferSize];
    unsigned int mBufFillSize;
    /** HTTP connection parameters */
    char mParams[CHttpComLayer::kAllocSize];
    };

  }

}

#endif /*_HTTPIPCOMLAYER_H_*/