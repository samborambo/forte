/*******************************************************************************
 * Copyright (c) 2016 Johannes Messmer (admin@jomess.com)
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Johannes Messmer - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "bus.h"

#include <algorithm>
#include <slave/slave.h>
#include <slave/packages.h>

namespace EmBrick {

DEFINE_SINGLETON(BusHandler)

BusHandler::BusHandler() :
    nextLoop(0), lastTransfer(0), spi(NULL), slaveSelect(NULL), slaves(NULL) {
  // Set init time
  struct timespec ts;
  // TODO Check compile error. Had to to add rt libary to c++ make flags
  clock_gettime(CLOCK_REALTIME, &ts);

  initTime = ts.tv_sec;
  lastTransfer = micros();
}

BusHandler::~BusHandler() {
}

void BusHandler::run() {
  // TODO Check status of handler errors
  DEVLOG_INFO("emBrick[BusHandler]: Handlers ready.\n");

  // Init bus
  init();

  unsigned long ms, diff;
  while (isAlive()) {
    ms = millis();
    if (nextLoop == 0)
      nextLoop = ms + (100 - ms % 100);

    while (nextLoop > ms) {
      microsleep(std::min((nextLoop - ms), (unsigned long) 2) * 1000);

      // TODO Check for updates of forte runtime -> maybe schedule next loop earlier
      // DISCUSS Use cond_timed_wait pthread to wake up thread
      ms = millis();
    }

    // TODO Perform requests to slaves
    // DISCUSS Use scheduling strategy or a cyclic routine
    TSlaveList::Iterator itEnd(slaves->end());
    for (TSlaveList::Iterator it = slaves->begin(); it != itEnd; ++it) {
      (*it)->update();
    }

    // Sleep till next cycle - pause at least a certain time to avoid blocking of the forte process
    diff = millis() - ms;
//		DEVLOG_INFO("emBrick[BusHandler]: Loop %d - %d\n", ms, diff);
    ms = millis();
    nextLoop = ms + std::max(20 - ms % 20, (unsigned long) 5);
  }

  DEVLOG_INFO("emBrick[BusHandler]: Last loop duration - %d\n", diff);

  // Free memory
  TSlaveList::Iterator itEnd(slaves->end());
  for (TSlaveList::Iterator it = slaves->begin(); it != itEnd; ++it)
    delete *it;
  delete spi;
  delete slaveSelect;

  DEVLOG_INFO("emBrick[BusHandler]: Stopped.\n");
}

void BusHandler::init() {
  // Start handlers
  spi = new SPIHandler();
  slaveSelect = new PinHandler(49);
  slaves = new TSlaveList();

  // Disable slave select -> reset all slaves
  slaveSelect->disable();

  // Wait for reset
  microsleep(SyncGapDuration * 2);

  // Enable slave select -> the first slave waits for initialization
  slaveSelect->enable();

  // Wait for init
  microsleep(SyncGapDuration * 2);

  // Init the slaves sequentially. Abort if the init package is ignored -> no further slaves found.
  int slaveCounter = 1;
  int attempts = 0;
  Slave *slave = NULL;

  do {
    slave = Slave::sendInit(slaveCounter);

    if (slave != NULL) {
      slaves->push_back(slave);

      // Activate next slave by sending the 'SelectNextSlave' command to the current slave
      // It enables the slave select pin for the next slave on the bus
      transfer(slave->address, SelectNextSlave);

      slaveCounter++;
      attempts = 0;
    }

    microsleep(SyncGapDuration * 2);
  } while (++attempts < 3);
}

bool BusHandler::transfer(unsigned int target, Command cmd,
    unsigned char* dataSend, int dataSendLength, unsigned char* dataReceive,
    int dataReceiveLength, SlaveStatus* status) {
  int dataLength = std::max(dataSendLength, dataReceiveLength + 1); // + 1 status byte

  int bufferLength = sizeof(Packages::Header) + dataLength + 1; // + 1 checksum byte
  unsigned char buffer[bufferLength];
  unsigned char recvBuffer[bufferLength];
  memset(buffer, 0, bufferLength);
  memset(recvBuffer, 0, bufferLength);

  Packages::Header* header = (Packages::Header*) buffer;

  header->address = (char) target;
  header->command = cmd;
  header->checksum = calcChecksum((unsigned char*) header, 2);

  memcpy(buffer + sizeof(Packages::Header), dataSend, dataSendLength);
  buffer[sizeof(Packages::Header) + dataSendLength] = calcChecksum(dataSend,
      dataSendLength);

//	DEVLOG_INFO("emBrick[BusHandler]: TX - %s\n",
//			bytesToHex(buffer, bufferLength).c_str());

  // Invert data of master
  for (int i = 0; i < bufferLength; i++)
    buffer[i] = ~buffer[i];

  // Send and receive buffer via SPI
  int attempts = 10;
  int fails = 0;
  bool ok;
  uint64_t microTime;
  do {
    // Wait required microseconds between messages
    microTime = micros();
    if (lastTransfer + SyncGapDuration > microTime)
      microsleep(lastTransfer + SyncGapDuration - microTime);

    ok = spi->transfer(buffer, recvBuffer, bufferLength);
    lastTransfer = micros();
    if (!ok) {
      DEVLOG_ERROR("emBrick[BusHandler]: Failed to transfer buffer.\n");
      break;
    }

//		DEVLOG_INFO("emBrick[BusHandler]: RX - %s, CS - %d\n",
//				bytesToHex(recvBuffer, bufferLength).c_str(),
//				calcChecksum(recvBuffer, bufferLength));

    ok = calcChecksum(recvBuffer, bufferLength) == 0;
    if (!ok) {
      DEVLOG_DEBUG("emBrick[BusHandler]: Transfer - Invalid checksum\n");
    }
  } while (!ok && ++fails < attempts);

  if (!ok) {
    DEVLOG_ERROR(
        "emBrick[BusHandler]: Failed to send command %d to slave %d.\n", cmd,
        target);
    return false;
  }

  // Copy result
  if (status)
    *status = (SlaveStatus) recvBuffer[0];

  memcpy(dataReceive, recvBuffer + sizeof(Packages::Header) + 1,
      dataReceiveLength);

  return true;
}

unsigned char BusHandler::calcChecksum(unsigned char * data, int dataLen) {
  unsigned char c = 0;
  for (int i = 0; i < dataLen; i++)
    c += data[i];

  return ChecksumConstant - c;
}

void BusHandler::disableHandler() {

}

void BusHandler::enableHandler() {

}

void BusHandler::setPriority(int) {

}

int BusHandler::getPriority() const {
  return 0;
}

uint64_t BusHandler::micros() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  return round(ts.tv_nsec / 1.0e3) + (ts.tv_sec - initTime) * 1E6;
}

unsigned long BusHandler::millis() {
  // TODO Improve timing func. Maybe replace with existing forte implementation.
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  return round(ts.tv_nsec / 1.0e6) + (ts.tv_sec - initTime) * 1000;
}

void BusHandler::microsleep(unsigned long microseconds) {
  struct timespec ts;
  ts.tv_sec = microseconds / (unsigned long) 1E6;
  ts.tv_nsec = microseconds * 1E3 - ts.tv_sec * 1E9;
  nanosleep(&ts, NULL);
}

std::string BusHandler::bytesToHex(unsigned char* bytes, int length) {
  // TODO Move helper functions to helper class or general forte utils class
  char buffer[length * 3];
  buffer[length * 3 - 1] = 0;
  for (int i = 0; i < length; i++)
    sprintf(&buffer[3 * i], "%02X ", bytes[i]);

  return buffer;
}

} /* namespace EmBrick */
