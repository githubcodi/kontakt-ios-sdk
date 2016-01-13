//
//  KontaktSDK
//  Version: 1.0.0
//
//  Copyright (c) 2015 Kontakt.io. All rights reserved.
//

@import Foundation;
@import CoreBluetooth;
@import CoreLocation;

#import "KTKNearbyDevice.h"
#import "KTKFirmware.h"
#import "KTKDeviceCredentials.h"
#import "KTKDeviceConfiguration.h"

NS_ASSUME_NONNULL_BEGIN

/**
 *  Device Connection Operation Types
 */
typedef NS_ENUM(NSInteger, KTKDeviceConnectionOperationType) {
    /**
     *  Type Unknown.
     */
    KTKDeviceConnectionOperationTypeUnknown = -1,
    /**
     *  Read Operation.
     */
    KTKDeviceConnectionOperationTypeRead    = 1,
    /**
     *  Write Operation.
     */
    KTKDeviceConnectionOperationTypeWrite   = 2,
    /**
     *  DFU Operation.
     */
    KTKDeviceConnectionOperationTypeDFU     = 3
};

#pragma mark - Completion Blocks
/**
 *  A completion block object to be executed when the write operation finishes.
 *
 *  @param synchronized  A Boolean indicating whether the configuration was synchronized to the Cloud API.
 *  @param configuration The configuration object containing write operation details. Please not that configuration object will be nil for devices with firmware < 4.0.
 *                       If <code>synchronized</code> is false you have to keep copy of the configuration and synchronize it with the cloud when possible.
 *                       When the device firmware is 4.0 or higher configuration object will contain <code>secureResponse</code> and <code>secureResponseTime</code>.
 *  @param error         An error object containing the error that indicates why the operation failed.
 */
typedef void (^KTKDeviceConnectionWriteCompletion)(BOOL synchronized, KTKDeviceConfiguration * _Nullable configuration, NSError * _Nullable error);

/**
 *  A completion block object to be executed when the read operation finishes.
 *
 *  @param configuration The configuration object. Result of the read operation.
 *  @param error         An error object containing the error that indicates why the operation failed.
 */
typedef void (^KTKDeviceConnectionReadCompletion)(KTKDeviceConfiguration * _Nullable configuration, NSError * _Nullable error);

/**
 *  A completion block object to be executed when the update operation finishes.
 *
 *  @param synchronized A Boolean indicating whether the configuration was synchronized to the Cloud API.
 *  @param error        An error object containing the error that indicates why the operation failed.
 */
typedef void (^KTKDeviceConnectionUpdateCompletion)(BOOL synchronized, NSError * _Nullable error);

@protocol KTKDeviceConnectionDelegate;

#pragma mark - KTKDeviceConnection (Interface)
@interface KTKDeviceConnection : NSObject

#pragma mark - Connection Properties
///--------------------------------------------------------------------
/// @name Connection Properties
///--------------------------------------------------------------------

/**
 *  A timeout interval for the connection.
 */
@property (nonatomic, assign, readwrite) NSTimeInterval connectionTimeout;

/**
 *  A number of maxium connection attempts if one failed.
 */
@property (nonatomic, assign, readwrite) NSUInteger connectionAttempts;

/**
 *  The delegate object that will receive events.
 */
@property (nonatomic, weak, readwrite) id<KTKDeviceConnectionDelegate> delegate;

#pragma mark - Other Properties
///--------------------------------------------------------------------
/// @name Other Properties
///--------------------------------------------------------------------

/**
 *  A credentails object to be forced over Cloud API credentials.
 *
 *  This property should be only used to connect to legacy devices (fw < 4.0) while offline.
 *  Devices with firmware >= 4.0 are not using password for the connection so this property will be ignored.
 *  By default credentials are automatically fetched from the Cloud API just before the connection opertation.
 *  If connection to the device while offline is not required this property should be ignored.
 *
 *  If password is changed through <code>KTKDeviceConfiguration</code> on the device while in offline it must be stored
 *  and then synchronized to the cloud API when possible.
 */
@property (nonatomic, copy, readwrite) KTKDeviceCredentials *credentials;

#pragma mark - Initialization Methods
///--------------------------------------------------------------------
/// @name Initialization Methods
///--------------------------------------------------------------------

/**
 *  Creates instance of KTKDeviceConnection with given KTKNearbyDevice
 *
 *  @param device KTKNearbyDevice Object
 *
 *  @return New instance of KTKDeviceConnection
 */
- (instancetype)initWithNearbyDevice:(KTKNearbyDevice*)nearbyDevice;

#pragma mark - Connection Methods
///--------------------------------------------------------------------
/// @name Connection Methods
///--------------------------------------------------------------------

/**
 *  Writes the specified configuration to the connection device.
 *
 *  @param configuration The configuration object to write.
 *  @param completion    A block object to be executed when the write operation finishes.
 */
- (void)writeConfiguration:(KTKDeviceConfiguration*)configuration completion:(KTKDeviceConnectionWriteCompletion)completion;

/**
 *  Reads the configuration from the connection device.
 *
 *  @param completion A block object to be executed when the read operation finishes.
 */
- (void)readConfigurationWithCompletion:(KTKDeviceConnectionReadCompletion)completion;

/**
 *  Updates a device to the latest available firmware.
 *
 *  @param firmware   The firmware object you want to update device to. <code>Has to be equal or grater than current firmware version.</code>
 *  @param progress   A block object to be executed when the progress of an update operation is recieved.
 *  @param completion A block object to be executed when the update operation finishes.
 */
- (void)updateWithFirmware:(KTKFirmware*)firmware progress:(void (^)(double))progress completion:(KTKDeviceConnectionUpdateCompletion)completion;

/**
 *  Cancels all operations on this connection.
 */
- (void)cancel;

@end

#pragma mark - KTKDeviceConnectionDelegate Protocol
/**
 *  It is possible if read and/or write operations are scheduled one after another that connection will happen only once,
 *  so any delegate's methods would be called just once, even with few configuration operations scheduled.
 */
@protocol KTKDeviceConnectionDelegate <NSObject>

#pragma mark - Connection Delegate Methods
///--------------------------------------------------------------------
/// @name Connection Delegate Methods
///--------------------------------------------------------------------

/**
 *  Tells the delegate that a device connection was successful.
 *
 *  @param connection The device connection object reporting the event.
 */
@optional
- (void)deviceConnectionDidConnect:(KTKDeviceConnection *)connection;

/**
 *  Tells the delegate what is the progress of a read or write operation.
 *
 *  @param connection The device connection object reporting the event.
 *  @param operation  An operation type.
 *  @param progress   An operation type.
 */
@optional
- (void)deviceConnection:(KTKDeviceConnection *)connection operation:(KTKDeviceConnectionOperationType)operation progress:(double)progress;

/**
 *  Tells the delegate that a device connection error occurred.
 *
 *  @param connection   The device connection object reporting the event.
 *  @param error        An error object containing the error that indicates why the connection failed.
 *  @param attemptsLeft A number of reconnection attempts left. If <code>attemptsLeft</code> is 0, it will report final error.
 */
@optional
- (void)deviceConnection:(KTKDeviceConnection *)connection didFailWithError:(NSError * _Nullable)error connectionAttemptsLeft:(NSInteger)attemptsLeft;

/**
 *  Tells the delegate that a devices connection was ended.
 *
 *  @param connection The device connection object reporting the event.
 *  @param error      An error object containing the error that indicates why the device disconnected.
 */
@optional
- (void)deviceConnectionDidDisconnect:(KTKDeviceConnection *)connection withError:(NSError * _Nullable)error;

@end

NS_ASSUME_NONNULL_END
