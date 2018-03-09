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

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTException;

/**
 * the RTRemoteTransport tcp implement
 */
public class RTRemoteTCPTransport implements RTRemoteTransport, Runnable {

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTRemoteTCPTransport.class.getName());

  /**
   * the tcp socket
   */
  private Socket socket;

  /**
   * the remote address
   */
  private InetAddress mRemoteAddress;

  /**
   * the local address
   */
  private InetAddress mLocalAddress;

  /**
   * the remote port
   */
  private int mRemotePort;

  /**
   * the socket data output stream
   */
  private DataOutputStream dataOutputStream;

  /**
   * the socket data input stream
   */
  private DataInputStream dataInputStream;

  /**
   * the read stream
   */
  private Thread mReaderthread;

  /**
   * is the tranport running
   */
  private boolean mRunning;

  /**
   * the bytes queue
   */
  private BlockingQueue<byte[]> blockingQueue;

  /**
   * create new RTRemoteTCPTransport
   *
   * @param remote the remote address
   * @param port the remote port
   * @throws RTException if any other error occurred during operation
   */
  public RTRemoteTCPTransport(String remote, int port) throws RTException {
    try {
      mRemoteAddress = InetAddress.getByName(remote);
    } catch (UnknownHostException err) {
      throw new RTException("failed to lookup " + remote, err);
    }
    mRemotePort = port;
    mLocalAddress = null;
    blockingQueue = new LinkedBlockingQueue<byte[]>();
  }

  public RTRemoteTCPTransport(Socket socket) throws RTException {
    this.socket = socket;
    blockingQueue = new LinkedBlockingQueue<byte[]>();
    try {
      bindSocketStream();
      start();
    } catch (IOException e) {
      logger.error(e);
      throw new RTException(e);
    }
  }

  /**
   * the thread that to read data
   */
  public void run() {
    try {
      while (mRunning) {
        int len = dataInputStream.readInt();
        byte[] buff = new byte[len];
        dataInputStream.readFully(buff, 0, len);
        try {
          blockingQueue.put(buff);
        } catch (InterruptedException err) {
          logger.warn("error reading from socket", err);
        }
      }
    } catch (IOException err) {
      logger.error("socket read error, maybe some client disconnect from server", err);
      try {
        close();
      } catch (Exception ignored) {
      }
    }
  }

  /**
   * open tcp socket
   *
   * @throws RTException @throws RTException if any other error occurred during operation
   */
  public void open() throws RTException {
    this.close();

    try {
      socket = new Socket(mRemoteAddress, mRemotePort);
      bindSocketStream();
    } catch (IOException err) {
      throw new RTException("failed to open socket", err);
    }

    logger.info("new tcp connection from: " + socket.getLocalAddress() + " to: " + socket
        .getRemoteSocketAddress());

    start();
  }

  /**
   * bind input/output stream to socket
   *
   * @throws IOException throw exception if bind failed
   */
  private void bindSocketStream() throws IOException {
    dataOutputStream = new DataOutputStream(socket.getOutputStream());
    dataInputStream = new DataInputStream(socket.getInputStream());
  }

  /**
   * start transport
   */
  private void start() {
    mRunning = true;
    mReaderthread = new Thread(this);
    mReaderthread.start();
  }

  /**
   * close tcp socket
   *
   * @throws RTException if any other error occurred during operation
   */
  public void close() throws RTException {
    if (socket != null) {
      try {
        socket.close();
      } catch (IOException ioe) {
        throw new RTException("failed to close socket", ioe);
      }
      socket = null;
    }
    if (mReaderthread != null) {
      // TODO
    }

    mRunning = false;
  }

  /**
   * read bytes
   *
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] recv() throws RTException {
    try {
      return blockingQueue.take();
    } catch (InterruptedException err) {
      throw new RTException(err);
    }
  }

  /**
   * send bytes to remote
   *
   * @param buff the bytes
   * @throws RTException if any other error occurred during operation
   */
  public void send(byte[] buff) throws RTException {
    try {
      dataOutputStream.writeInt(buff.length);
      dataOutputStream.write(buff, 0, buff.length);
      dataOutputStream.flush();
    } catch (IOException ioe) {
      throw new RTException("error sending message", ioe);
    }
  }
}
