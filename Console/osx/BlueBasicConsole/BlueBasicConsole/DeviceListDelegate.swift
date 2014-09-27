//
//  DeviceListDelegate.swift
//  BlueBasicConsole
//
//  Created by tim on 9/24/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation

protocol DeviceListDelegate {
  func onDeviceConnect(device: Device)
  func onDeviceDisconnect(device: Device)
}
