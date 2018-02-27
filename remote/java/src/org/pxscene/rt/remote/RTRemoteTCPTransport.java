//
// Copyright [2018] [jacobgladish@yahoo.com]
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
package org.pxscene.rt.remote;

import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.pxscene.rt.RTException;

public class RTRemoteTCPTransport implements RTRemoteTransport, Runnable {
  private static final Logger log = Logger.getLogger(RTRemoteTCPTransport.class.getName());

  private Socket m_soc;
  private InetAddress m_remoteAddress;
  private InetAddress m_localAddress;
  private int m_remotePort;
  private DataOutputStream m_out;
  private DataInputStream m_in;
  private Thread m_readerThread;
  private boolean m_running;
  private BlockingQueue<byte[]> m_incomingQueue;

  public RTRemoteTCPTransport(String remote, int port) throws RTException {
    try {
      m_remoteAddress = InetAddress.getByName(remote);
    }
    catch (UnknownHostException err) {
      throw new RTException("failed to lookup " + remote, err);
    }
    m_remotePort = port;
    m_localAddress = null;
    m_incomingQueue = new LinkedBlockingQueue<byte []>();
  }

  public void run() {
    try {
      while (m_running) {
        int len = m_in.readInt();
        byte[] buff = new byte[len];
        m_in.readFully(buff, 0, len);
        try {
          m_incomingQueue.put(buff);
        } catch (InterruptedException err) {
          log.log(Level.WARNING, "error reading from socket", err);
        }
      }
    }
    catch (IOException err) {
      // TODO log exception
      try { close(); }
      catch (Exception ignored) { }
    }
  }

  public void open() throws RTException {
    this.close();

    try {
      m_soc = new Socket(m_remoteAddress, m_remotePort);
      m_out = new DataOutputStream(m_soc.getOutputStream());
      m_in = new DataInputStream(m_soc.getInputStream());
    }
    catch (IOException err) {
      throw new RTException("failed to open socket", err);
    }

    if (log.isLoggable(Level.INFO))
      log.info("new tcp connection from: " + m_soc.getLocalAddress() + " to: " + m_soc.getRemoteSocketAddress());

    m_running = true;
    m_readerThread = new Thread(this);
    m_readerThread.start();
  }

  public void close() throws RTException {
    if (m_soc != null) {
      try {
        m_soc.close();
      }
      catch (IOException ioe) {
        throw new RTException("failed to close socket", ioe);
      }
      m_soc = null;
    }
    if (m_readerThread != null) {
      // TODO
    }

    m_running = false;
  }

  public byte[] recv() throws RTException {
    try {
      return m_incomingQueue.take();
    } catch (InterruptedException err) {
      throw new RTException(err);
    }
  }

  public void send(byte[] buff) throws RTException {
    try {
      m_out.writeInt(buff.length);
      m_out.write(buff, 0, buff.length);
      m_out.flush();
    }
    catch (IOException ioe) {
      throw new RTException("error sending message", ioe);
    }
  }
}
