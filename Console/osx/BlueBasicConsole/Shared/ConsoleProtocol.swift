//
//  ConsoleProtocol.swift
//  BlueBasicConsole
//
//  Created by tim on 10/8/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation

protocol ConsoleProtocol {
  
  var status: String { get set }
  
  func setStatus(status: String) // Workaround
  
  var delegate: ConsoleDelegate? { get set }
  
  func setDelegate(delegate: ConsoleDelegate)
  
  var current: Device? { get }
  
  var isConnected: Bool { get }
  
  var isRecoveryMode: Bool { get }
  
  func connectTo(device: Device, onConnected: CompletionHandler?)
  
  func disconnect(onDisconnect: CompletionHandler?)
  
  func write(str: String)

}