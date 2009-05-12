/*
 * osRgstry.c
 *
 * Copyright(c) 1998 - 2009 Texas Instruments. All rights reserved.      
 * All rights reserved.                                                  
 *                                                                       
 * Redistribution and use in source and binary forms, with or without    
 * modification, are permitted provided that the following conditions    
 * are met:                                                              
 *                                                                       
 *  * Redistributions of source code must retain the above copyright     
 *    notice, this list of conditions and the following disclaimer.      
 *  * Redistributions in binary form must reproduce the above copyright  
 *    notice, this list of conditions and the following disclaimer in    
 *    the documentation and/or other materials provided with the         
 *    distribution.                                                      
 *  * Neither the name Texas Instruments nor the names of its            
 *    contributors may be used to endorse or promote products derived    
 *    from this software without specific prior written permission.      
 *                                                                       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#if defined(_WINDOWS)
#include <ndis.h>
#elif defined( __LINUX__ )
#include "osRgstry_parser.h"
#elif defined(__ARMCC__) 
#include "osRgstry_parser.h"
#include "string.h"
#endif
#include "WlanDrvIf.h"
#include "osRgstry.h"
#include "paramOut.h"
#include "osDot11.h"
#include "osApi.h"
#include "rate.h"
#include "802_11Defs.h"



#define MAX_KEY_BUFFER_LEN      256
#define BIT_TO_BYTE_FACTOR      8

#define N_STR(str)              NDIS_STRING_CONST(str)
#define INIT_TBL_OFF(field)     FIELD_OFFSET(TInitTable, field)

/* Reports */
NDIS_STRING STR_ReportSeverityTable          = NDIS_STRING_CONST( "ReportSeverityTable" );
NDIS_STRING STR_ReportModuleTable            = NDIS_STRING_CONST( "ReportModuleTable" );


NDIS_STRING STRFilterEnabled            = NDIS_STRING_CONST( "Mac_Filter_Enabled");
NDIS_STRING STRnumGroupAddrs            = NDIS_STRING_CONST( "numGroupAddrs" );
NDIS_STRING STRGroup_addr0              = NDIS_STRING_CONST( "Group_addr0" );
NDIS_STRING STRGroup_addr1              = NDIS_STRING_CONST( "Group_addr1" );
NDIS_STRING STRGroup_addr2              = NDIS_STRING_CONST( "Group_addr2" );
NDIS_STRING STRGroup_addr3              = NDIS_STRING_CONST( "Group_addr3" );
NDIS_STRING STRGroup_addr4              = NDIS_STRING_CONST( "Group_addr4" );
NDIS_STRING STRGroup_addr5              = NDIS_STRING_CONST( "Group_addr5" );
NDIS_STRING STRGroup_addr6              = NDIS_STRING_CONST( "Group_addr6" );
NDIS_STRING STRGroup_addr7              = NDIS_STRING_CONST( "Group_addr7" );

/* Beacon timing */
/* If Early Wakeup is Enabled, 1251 wakes-up EARLY_WAKEUP_TIME before expected Beacon reception occasion */
/* If Early Wakeup is Disabled, 1251 wakes-up at the expected Beacon reception occasion. */
NDIS_STRING STREarlyWakeup                 = NDIS_STRING_CONST( "EarlyWakeup" );
    
NDIS_STRING STRArp_Ip_Addr              = NDIS_STRING_CONST( "ArpIp_Addr" );
NDIS_STRING STRArp_Ip_Filter_Ena        = NDIS_STRING_CONST( "ArpIp_Filter_ena");


NDIS_STRING STRBeaconFilterDesiredState = NDIS_STRING_CONST( "Beacon_Filter_Desired_State") ;
NDIS_STRING STRBeaconFilterStored       = NDIS_STRING_CONST( "Beacon_Filter_Stored") ;  

/*this is for configuring table from ini file*/
NDIS_STRING STRBeaconIETableSize            = NDIS_STRING_CONST( "Beacon_IE_Table_Size") ;  
NDIS_STRING STRBeaconIETable                = NDIS_STRING_CONST( "Beacon_IE_Table") ;
NDIS_STRING STRBeaconIETableNumOfElem       = NDIS_STRING_CONST( "Beacon_IE_Num_Of_Elem") ; 

NDIS_STRING STRCoexActivityTable                = NDIS_STRING_CONST( "Coex_Activity_Table") ;
NDIS_STRING STRCoexActivityNumOfElem            = NDIS_STRING_CONST( "Coex_Activity_Num_Of_Elem") ; 

/* ------------------------------------------------------ */
NDIS_STRING STRFirmwareDebug                = NDIS_STRING_CONST( "FirmwareDebug" );
NDIS_STRING STRTraceBufferSize              = NDIS_STRING_CONST( "TraceBufferSize" );
NDIS_STRING STRPrintTrace                   = NDIS_STRING_CONST( "PrintTrace" );

NDIS_STRING STRHwACXAccessMethod            = NDIS_STRING_CONST( "HwACXAccessMethod" );
NDIS_STRING STRMaxSitesFragCollect          = NDIS_STRING_CONST( "MaxSitesFragCollect" );

NDIS_STRING STRTxFlashEnable                = NDIS_STRING_CONST( "TxFlashEnable" );

NDIS_STRING STRBetEnable                    = NDIS_STRING_CONST( "BetEnable");
NDIS_STRING STRBetMaxConsecutive            = NDIS_STRING_CONST( "BetMaxConsecutive");
NDIS_STRING STRMaxFullBeaconInterval        = NDIS_STRING_CONST( "MaxlFullBeaconReceptionInterval" );
NDIS_STRING STRBetEnableThreshold           = NDIS_STRING_CONST( "BetEnableThreshold");
NDIS_STRING STRBetDisableThreshold          = NDIS_STRING_CONST( "BetDisableThreshold");

NDIS_STRING STRNumHostRxDescriptors         = NDIS_STRING_CONST( "NumHostRxDescriptors" );
NDIS_STRING STRNumHostTxDescriptors         = NDIS_STRING_CONST( "NumHostTxDescriptors" );

NDIS_STRING STRACXMemoryBlockSize           = NDIS_STRING_CONST( "ACXMemoryBlockSize" );
NDIS_STRING STRACXRxMemoryBlockSize         = NDIS_STRING_CONST( "ACXMemoryBlockSize" );
NDIS_STRING STRACXTxMemoryBlockSize         = NDIS_STRING_CONST( "ACXMemoryBlockSize" );

NDIS_STRING STRACXUseTxDataInterrupt        = NDIS_STRING_CONST( "ACXUseTxDataInterrupt" );
NDIS_STRING STRACXUseInterruptThreshold     = NDIS_STRING_CONST( "ACXUseInterruptThreshold" );

NDIS_STRING STRCalibrationChannel2_4        = NDIS_STRING_CONST( "CalibrationChannel24" );
NDIS_STRING STRCalibrationChannel5_0        = NDIS_STRING_CONST( "CalibrationChannel5" );
NDIS_STRING STRdot11RTSThreshold            = NDIS_STRING_CONST( "dot11RTSThreshold" );
NDIS_STRING STRRxDisableBroadcast           = NDIS_STRING_CONST( "RxDisableBroadcast" );
NDIS_STRING STRRecoveryEnable               = NDIS_STRING_CONST( "RecoveryEnable" );
NDIS_STRING STRdot11TxAntenna               = NDIS_STRING_CONST( "dot11TxAntenna" );
NDIS_STRING STRdot11RxAntenna               = NDIS_STRING_CONST( "dot11RxAntenna" );

NDIS_STRING STRTxCompleteThreshold          = NDIS_STRING_CONST( "TxCompleteThreshold" );
NDIS_STRING STRTxCompleteTimeout            = NDIS_STRING_CONST( "TxCompleteTimeout" );
NDIS_STRING STRRxInterruptThreshold         = NDIS_STRING_CONST( "RxInterruptThreshold" );
NDIS_STRING STRRxInterruptTimeout           = NDIS_STRING_CONST( "RxInterruptTimeout" );

NDIS_STRING STRdot11FragThreshold           = NDIS_STRING_CONST( "dot11FragmentationThreshold" );
NDIS_STRING STRdot11MaxTxMSDULifetime       = NDIS_STRING_CONST( "dot11MaxTransmitMSDULifetime" );
NDIS_STRING STRdot11MaxReceiveLifetime      = NDIS_STRING_CONST( "dot11MaxReceiveLifetime" );
NDIS_STRING STRdot11RateFallBackRetryLimit  = NDIS_STRING_CONST( "dot11RateFallBackRetryLimit");

NDIS_STRING STRListenInterval               = NDIS_STRING_CONST( "dot11ListenInterval" );
NDIS_STRING STRExternalMode                 = NDIS_STRING_CONST( "DriverExternalMode" );
NDIS_STRING STRWiFiAdHoc                    = NDIS_STRING_CONST( "WiFiAdhoc" );
NDIS_STRING STRWiFiWmmPS                    = NDIS_STRING_CONST( "WiFiWmmPS" );
NDIS_STRING STRWiFiMode                     = NDIS_STRING_CONST( "WiFiMode" );
NDIS_STRING STRdot11DesiredChannel          = NDIS_STRING_CONST( "dot11DesiredChannel");
NDIS_STRING STRdot11DesiredSSID             = NDIS_STRING_CONST( "dot11DesiredSSID" );
NDIS_STRING STRdot11DesiredBSSType          = NDIS_STRING_CONST( "dot11DesiredBSSType" );
NDIS_STRING STRdot11BasicRateMask_B           = NDIS_STRING_CONST( "dot11BasicRateMaskB");
NDIS_STRING STRdot11SupportedRateMask_B       = NDIS_STRING_CONST( "dot11SupportedRateMaskB");
NDIS_STRING STRdot11BasicRateMask_G           = NDIS_STRING_CONST( "dot11BasicRateMaskG");
NDIS_STRING STRdot11SupportedRateMask_G       = NDIS_STRING_CONST( "dot11SupportedRateMaskG");
NDIS_STRING STRdot11BasicRateMask_A           = NDIS_STRING_CONST( "dot11BasicRateMaskA");
NDIS_STRING STRdot11SupportedRateMask_A       = NDIS_STRING_CONST( "dot11SupportedRateMaskA");
NDIS_STRING STRdot11BasicRateMask_AG           = NDIS_STRING_CONST( "dot11BasicRateMaskAG");
NDIS_STRING STRdot11SupportedRateMask_AG       = NDIS_STRING_CONST( "dot11SupportedRateMaskAG");
NDIS_STRING STRdot11BasicRateMask_N           = NDIS_STRING_CONST( "STRdot11BasicRateMask_N");
NDIS_STRING STRdot11SupportedRateMask_N       = NDIS_STRING_CONST( "STRdot11SupportedRateMask_N");

NDIS_STRING STRRadio11_RxLevel              = NDIS_STRING_CONST( "Radio11_RxLevel");
NDIS_STRING STRRadio11_LNA                  = NDIS_STRING_CONST( "Radio11_LNA");
NDIS_STRING STRRadio11_RSSI                 = NDIS_STRING_CONST( "Radio11_RSSI");
NDIS_STRING STRRadio0D_RxLevel              = NDIS_STRING_CONST( "Radio0D_RxLevel");
NDIS_STRING STRRadio0D_LNA                  = NDIS_STRING_CONST( "Radio0D_LNA");
NDIS_STRING STRRadio0D_RSSI                 = NDIS_STRING_CONST( "Radio0D_RSSI");

NDIS_STRING STRdot11DesiredNetworkType      = NDIS_STRING_CONST( "dot11NetworkType");
NDIS_STRING STRdot11DefaultNetworkType      = NDIS_STRING_CONST( "dot11DefaultNetworkType");
NDIS_STRING STRdot11SlotTime                = NDIS_STRING_CONST( "ShortSlotTime");
NDIS_STRING STRdot11IbssProtection          = NDIS_STRING_CONST( "IbssProtectionType");
NDIS_STRING STRdot11RtsCtsProtection        = NDIS_STRING_CONST( "dot11RtsCtsProtection");

NDIS_STRING STRRxEnergyDetection              = NDIS_STRING_CONST( "RxEnergyDetection" );
NDIS_STRING STRTxEnergyDetection              = NDIS_STRING_CONST( "TxEnergyDetection" );
NDIS_STRING STRCrtCalibrationInterval       = NDIS_STRING_CONST( "CrtCalibrationInterval" );
NDIS_STRING STRTddCalibrationInterval       = NDIS_STRING_CONST( "TddCalibrationInterval" );
NDIS_STRING STRMacClockRate                 = NDIS_STRING_CONST( "MacClockRate" );
NDIS_STRING STRArmClockRate                 = NDIS_STRING_CONST( "ArmClockRate" );
NDIS_STRING STRg80211DraftNumber            = NDIS_STRING_CONST( "g80211DraftNumber" );

NDIS_STRING STRdot11ShortPreambleInvoked    = NDIS_STRING_CONST( "dot11ShortPreambleInvoked" );

NDIS_STRING STRdot11BeaconPeriod            = NDIS_STRING_CONST( "dot11BeaconPeriod" );
NDIS_STRING STRdot11MaxScanTime             = NDIS_STRING_CONST( "dot11MaxScanTime" );
NDIS_STRING STRdot11MinScanTime             = NDIS_STRING_CONST( "dot11MinScanTime" );
NDIS_STRING STRdot11MaxSiteLifetime         = NDIS_STRING_CONST( "dot11MaxSiteLifetime" );

NDIS_STRING STRdot11MaxAuthRetry            = NDIS_STRING_CONST( "dot11MaxAuthRetry" );
NDIS_STRING STRdot11MaxAssocRetry           = NDIS_STRING_CONST( "dot11MaxAssocRetry" );
NDIS_STRING STRdot11AuthRespTimeout         = NDIS_STRING_CONST( "dot11AuthenticationResponseTimeout" );
NDIS_STRING STRdot11AssocRespTimeout        = NDIS_STRING_CONST( "dot11AssociationResponseTimeout" );

NDIS_STRING STRConnSelfTimeout              = NDIS_STRING_CONST( "ConnSelfTimeout" );

NDIS_STRING STRCreditCalcTimout             = NDIS_STRING_CONST( "CreditCalcTimout" );
NDIS_STRING STRCreditCalcTimerEnabled       = NDIS_STRING_CONST( "CreditCalcTimerEnabled" );

NDIS_STRING STRTrafficAdmControlTimeout     = NDIS_STRING_CONST("TrafficAdmControlTimeout");
NDIS_STRING STRTrafficAdmControlUseFixedMsduSize = NDIS_STRING_CONST("TrafficAdmCtrlUseFixedMsduSize");
NDIS_STRING STRDesiredMaxSpLen              = NDIS_STRING_CONST("DesiredMaxSpLen");

NDIS_STRING STRCwFromUserEnable             = NDIS_STRING_CONST("CwFromUserEnable");
NDIS_STRING STRDesireCwMin                  = NDIS_STRING_CONST("DesireCwMin");
NDIS_STRING STRDesireCwMax                  = NDIS_STRING_CONST("DesireCwMax");

NDIS_STRING STRRatePolicyUserShortRetryLimit   = NDIS_STRING_CONST( "RatePolicyUserShortRetryLimit" );
NDIS_STRING STRRatePolicyUserLongRetryLimit    = NDIS_STRING_CONST( "RatePolicyUserLongRetryLimit" );

NDIS_STRING STRRatePolicyUserEnabledRatesMaskCck     = NDIS_STRING_CONST( "RatePolicyUserEnabledRatesMaskCck" );
NDIS_STRING STRRatePolicyUserEnabledRatesMaskOfdm    = NDIS_STRING_CONST( "RatePolicyUserEnabledRatesMaskOfdm" );
NDIS_STRING STRRatePolicyUserEnabledRatesMaskOfdmA   = NDIS_STRING_CONST( "RatePolicyUserEnabledRatesMaskOfdmA" );
NDIS_STRING STRRatePolicyUserEnabledRatesMaskOfdmN   = NDIS_STRING_CONST( "RatePolicyUserEnabledRatesMaskOfdmN" );

NDIS_STRING STRdot11AuthenticationMode      = NDIS_STRING_CONST( "dot11AuthenticationMode" );
NDIS_STRING STRdot11WEPStatus               = NDIS_STRING_CONST( "dot11WEPStatus" );
NDIS_STRING STRdot11ExcludeUnencrypted      = NDIS_STRING_CONST( "dot11ExcludeUnencrypted" );
NDIS_STRING STRdot11WEPKeymappingLength     = NDIS_STRING_CONST( "dot11WEPKeymappingLength" );
NDIS_STRING STRdot11WEPDefaultKeyID         = NDIS_STRING_CONST( "dot11WEPDefaultKeyID" );

NDIS_STRING STRMixedMode                    = NDIS_STRING_CONST( "MixedMode" );

NDIS_STRING STRWPAMixedMode                  = NDIS_STRING_CONST( "WPAMixedMode");
NDIS_STRING STRRSNPreAuth                    = NDIS_STRING_CONST( "RSNPreAuthentication");
NDIS_STRING STRRSNPreAuthTimeout             = NDIS_STRING_CONST( "RSNPreAuthTimeout" );

NDIS_STRING STRPairwiseMicFailureFilter		= NDIS_STRING_CONST( "PairwiseMicFailureFilter" );

NDIS_STRING STRTimeToResetCountryMs         = NDIS_STRING_CONST( "TimeToResetCountryMs" );
NDIS_STRING STRMultiRegulatoryDomainEnabled = NDIS_STRING_CONST( "MultiRegulatoryDomain" );
NDIS_STRING STRSpectrumManagementEnabled    = NDIS_STRING_CONST( "SpectrumManagement" );
NDIS_STRING STRScanControlTable24           = NDIS_STRING_CONST( "AllowedChannelsTable24" );
NDIS_STRING STRScanControlTable5            = NDIS_STRING_CONST( "AllowedChannelsTable5" );

NDIS_STRING STRBurstModeEnable              = NDIS_STRING_CONST( "BurstModeEnable" );

/*
Power Manager
*/
NDIS_STRING STRPowerMode                    = NDIS_STRING_CONST( "dot11PowerMode" );
NDIS_STRING STRBeaconReceiveTime            = NDIS_STRING_CONST( "BeaconReceiveTime" );
NDIS_STRING STRBaseBandWakeUpTime           = NDIS_STRING_CONST( "BaseBandWakeUpTime" );
NDIS_STRING STRHangoverPeriod               = NDIS_STRING_CONST( "HangoverPeriod" );
NDIS_STRING STRBeaconListenInterval         = NDIS_STRING_CONST( "BeaconListenInterval" );
NDIS_STRING STRDtimListenInterval         = NDIS_STRING_CONST( "DtimListenInterval" );
NDIS_STRING STRNConsecutiveBeaconsMissed    = NDIS_STRING_CONST( "NConsecutiveBeaconsMissed" );
NDIS_STRING STREnterTo802_11PsRetries       = NDIS_STRING_CONST( "EnterTo802_11PsRetries" );
NDIS_STRING STRAutoPowerModeInterval        = NDIS_STRING_CONST( "AutoPowerModeInterval" );
NDIS_STRING STRAutoPowerModeActiveTh        = NDIS_STRING_CONST( "AutoPowerModeActiveTh" );
NDIS_STRING STRAutoPowerModeDozeTh          = NDIS_STRING_CONST( "AutoPowerModeDozeTh" );
NDIS_STRING STRAutoPowerModeDozeMode        = NDIS_STRING_CONST( "AutoPowerModeDozeMode" );
NDIS_STRING STRDefaultPowerLevel            = NDIS_STRING_CONST( "defaultPowerLevel" );
NDIS_STRING STRPowerSavePowerLevel          = NDIS_STRING_CONST( "PowerSavePowerLevel" );
NDIS_STRING STRHostClkSettlingTime          = NDIS_STRING_CONST( "HostClkSettlingTime" );
NDIS_STRING STRHostFastWakeupSupport        = NDIS_STRING_CONST( "HostFastWakeupSupport" );

NDIS_STRING STRPsPollDeliveryFailureRecoveryPeriod     = NDIS_STRING_CONST( "PsPollDeliveryFailureRecoveryPeriod" );

NDIS_STRING STRPowerMgmtHangOverPeriod      = NDIS_STRING_CONST( "PowerMgmtHangOverPeriod" );
NDIS_STRING STRPowerMgmtMode                = NDIS_STRING_CONST( "PowerMgmtMode" );
NDIS_STRING STRPowerMgmtNeedToSendNullData  = NDIS_STRING_CONST( "PowerMgmtNeedToSendNullData" );
NDIS_STRING STRPowerMgmtNullPktRateModulation = NDIS_STRING_CONST( "PowerMgmtNullPktRateModulation" );
NDIS_STRING STRPowerMgmtNumNullPktRetries   = NDIS_STRING_CONST( "PowerMgmtNumNullPktRetries" );
NDIS_STRING STRPowerMgmtPllLockTime         = NDIS_STRING_CONST( "PllLockTime" );

NDIS_STRING STRBeaconRxTimeout     = NDIS_STRING_CONST( "BeaconRxTimeout" );
NDIS_STRING STRBroadcastRxTimeout  = NDIS_STRING_CONST( "BroadcastRxTimeout" );
NDIS_STRING STRRxBroadcastInPs     = NDIS_STRING_CONST( "RxBroadcastInPs" );

NDIS_STRING STRConsecutivePsPollDeliveryFailureThreshold = NDIS_STRING_CONST( "ConsecutivePsPollDeliveryFailureThreshold" );

NDIS_STRING STRTxPower                      = NDIS_STRING_CONST( "TxPower" );

/* Scan SRV */
NDIS_STRING STRNumberOfNoScanCompleteToRecovery         = NDIS_STRING_CONST( "NumberOfNoScanCompleteToRecovery" );
NDIS_STRING STRTriggeredScanTimeOut                     = NDIS_STRING_CONST( "TriggeredScanTimeOut" );

/*-----------------------------------*/
/*   Bluetooth support               */
/*-----------------------------------*/
NDIS_STRING STRBThWlanCoexistEnable                     = NDIS_STRING_CONST( "BThWlanCoexistEnable" );

NDIS_STRING STRBThWlanCoexistParamsBtLoadRatio			= NDIS_STRING_CONST( "BThWlanCoexistLoadRatio" );
NDIS_STRING STRBThWlanCoexistParamsAutoPsMode			= NDIS_STRING_CONST( "BThWlanCoexistAutoPsMode" );
NDIS_STRING STRBThWlanCoexistParamsAutoScanProbeReqPerc	= NDIS_STRING_CONST( "BThWlanCoexistAutoScanProbeReqPerc" );
NDIS_STRING STRBThWlanCoexistParamsAutoScanWindowPerc	= NDIS_STRING_CONST( "BThWlanCoexistAutoScanWindowPerc" );
NDIS_STRING STRBThWlanCoexistParamsAutoScanMaxTime	    = NDIS_STRING_CONST( "BThWlanCoexistParamsAutoScanMaxTime" );
NDIS_STRING STRBThWlanCoexistPerThreshold	    		= NDIS_STRING_CONST( "BThWlanCoexistPerThreshold" );
NDIS_STRING STRBThWlanCoexistNfsSampleInterval      	= NDIS_STRING_CONST( "BThWlanCoexistNfsSampleInterval" );
NDIS_STRING STRBThWlanCoexistcoexAntennaConfiguration   = NDIS_STRING_CONST( "BThWlanCoexistcoexAntennaConfiguration" );
NDIS_STRING STRBThWlanCoexistcoexMaxConsecutiveBeaconMissPrecent   = NDIS_STRING_CONST( "BThWlanCoexMaxConsecutiveBeaconMissPrecent" );
NDIS_STRING STRBThWlanCoexistcoexAPRateAdapationThr   = NDIS_STRING_CONST( "BThWlanCoexistcoexAPRateAdapationThr" );
NDIS_STRING STRBThWlanCoexistcoexAPRateAdapationSnr   = NDIS_STRING_CONST( "BThWlanCoexistcoexAPRateAdapationSnr" );


NDIS_STRING STRDisableSsidPending           = NDIS_STRING_CONST( "DisableSsidPending" );

/*-----------------------------------*/
/*   SME Init Params                 */
/*-----------------------------------*/
NDIS_STRING STRSmeRadioOn                   = NDIS_STRING_CONST( "RadioOn" );
NDIS_STRING STRSmeConnectMode               = NDIS_STRING_CONST( "SmeConnectMode" );
NDIS_STRING STRSmeScanRssiThreshold         = NDIS_STRING_CONST( "SmeScanRssiThreshold" );
NDIS_STRING STRSmeScanSnrThreshold          = NDIS_STRING_CONST( "SmeScanSnrThreshold" );
NDIS_STRING STRSmeScanCycleNumber           = NDIS_STRING_CONST( "SmeScanCycleNumber" );
NDIS_STRING STRSmeScanMaxDwellTime          = NDIS_STRING_CONST( "SmeScanMaxDwellTimeMs" );
NDIS_STRING STRSmeScanMinDwellTime          = NDIS_STRING_CONST( "SmeScanMinDwellTimeMs" );
NDIS_STRING STRSmeScanProbeRequestNumber    = NDIS_STRING_CONST( "SmeScanProbeRequestNumber" );
NDIS_STRING STRSmeScanIntervals             = NDIS_STRING_CONST( "SmeScanIntervalList" );
NDIS_STRING STRSmeScanGChannels             = NDIS_STRING_CONST( "SmeScanGChannelList" );
NDIS_STRING STRSmeScanAChannels             = NDIS_STRING_CONST( "SmeScanAChannelList" );

/*-----------------------------------*/
/*   Health Check Init Params        */
/*-----------------------------------*/
NDIS_STRING STRRecoveryEnabledNoScanComplete    = NDIS_STRING_CONST( "RecoveryEnabledNoScanComplete" );
NDIS_STRING STRRecoveryEnabledMboxFailure       = NDIS_STRING_CONST( "RecoveryEnabledMboxFailure" );
NDIS_STRING STRRecoveryEnabledHwAwakeFailure    = NDIS_STRING_CONST( "RecoveryEnabledHwAwakeFailure" );
NDIS_STRING STRRecoveryEnabledTxStuck           = NDIS_STRING_CONST( "RecoveryEnabledTxStuck" );
NDIS_STRING STRRecoveryEnabledDisconnectTimeout = NDIS_STRING_CONST( "RecoveryEnabledDisconnectTimeout" );
NDIS_STRING STRRecoveryEnabledPowerSaveFailure  = NDIS_STRING_CONST( "RecoveryEnabledPowerSaveFailure" );
NDIS_STRING STRRecoveryEnabledMeasurementFailure= NDIS_STRING_CONST( "RecoveryEnabledMeasurementFailure" );
NDIS_STRING STRRecoveryEnabledBusFailure        = NDIS_STRING_CONST( "RecoveryEnabledBusFailure" );
NDIS_STRING STRRecoveryEnabledHwWdExpire        = NDIS_STRING_CONST( "RecoveryEnabledHwWdExpire" );

/*-----------------------------------*/
/* Tx Power control with atheros     */
/*-----------------------------------*/
NDIS_STRING STRTxPowerCheckTime                 = NDIS_STRING_CONST("TxPowerCheckTime");
NDIS_STRING STRTxPowerControlOn                 = NDIS_STRING_CONST("TxPowerControlOn");
NDIS_STRING STRTxPowerRssiThresh                = NDIS_STRING_CONST("TxPowerRssiThresh");
NDIS_STRING STRTxPowerRssiRestoreThresh         = NDIS_STRING_CONST("TxPowerRssiRestoreThresh");
NDIS_STRING STRTxPowerTempRecover              = NDIS_STRING_CONST("TxPowerTempRecover");


/*-----------------------------------*/
/*-----------------------------------*/
/*        QOS Parameters             */
/*-----------------------------------*/
NDIS_STRING STRWMEEnable                        = NDIS_STRING_CONST("WME_Enable");
NDIS_STRING STRTrafficAdmCtrlEnable             = NDIS_STRING_CONST("TrafficAdmCtrl_Enable");
NDIS_STRING STRdesiredPsMode                    = NDIS_STRING_CONST("desiredPsMode");
NDIS_STRING STRQOSmsduLifeTimeBE                = NDIS_STRING_CONST("QOS_msduLifeTimeBE");
NDIS_STRING STRQOSmsduLifeTimeBK                = NDIS_STRING_CONST("QOS_msduLifeTimeBK");
NDIS_STRING STRQOSmsduLifeTimeVI                = NDIS_STRING_CONST("QOS_msduLifeTimeVI");
NDIS_STRING STRQOSmsduLifeTimeVO                = NDIS_STRING_CONST("QOS_msduLifeTimeVO");
NDIS_STRING STRQOSrxTimeOutPsPoll               = NDIS_STRING_CONST("QOS_rxTimeoutPsPoll");
NDIS_STRING STRQOSrxTimeOutUPSD                 = NDIS_STRING_CONST("QOS_rxTimeoutUPSD");
NDIS_STRING STRQOSwmePsModeBE                   = NDIS_STRING_CONST("QOS_wmePsModeBE");
NDIS_STRING STRQOSwmePsModeBK                   = NDIS_STRING_CONST("QOS_wmePsModeBK");
NDIS_STRING STRQOSwmePsModeVI                   = NDIS_STRING_CONST("QOS_wmePsModeVI");
NDIS_STRING STRQOSwmePsModeVO                   = NDIS_STRING_CONST("QOS_wmePsModeVO");
NDIS_STRING STRQOSShortRetryLimitBE             = NDIS_STRING_CONST("QOS_ShortRetryLimitBE");
NDIS_STRING STRQOSShortRetryLimitBK             = NDIS_STRING_CONST("QOS_ShortRetryLimitBK");
NDIS_STRING STRQOSShortRetryLimitVI             = NDIS_STRING_CONST("QOS_ShortRetryLimitVI");
NDIS_STRING STRQOSShortRetryLimitVO             = NDIS_STRING_CONST("QOS_ShortRetryLimitVO");
NDIS_STRING STRQOSLongRetryLimitBE              = NDIS_STRING_CONST("QOS_LongRetryLimitBE");
NDIS_STRING STRQOSLongRetryLimitBK              = NDIS_STRING_CONST("QOS_LongRetryLimitBK");
NDIS_STRING STRQOSLongRetryLimitVI              = NDIS_STRING_CONST("QOS_LongRetryLimitVI");
NDIS_STRING STRQOSLongRetryLimitVO              = NDIS_STRING_CONST("QOS_LongRetryLimitVO");

NDIS_STRING STRQOSAckPolicyBE                   = NDIS_STRING_CONST("QOS_AckPolicyBE");
NDIS_STRING STRQOSAckPolicyBK                   = NDIS_STRING_CONST("QOS_AckPolicyBK");
NDIS_STRING STRQOSAckPolicyVI                   = NDIS_STRING_CONST("QOS_AckPolicyVI");
NDIS_STRING STRQOSAckPolicyVO                   = NDIS_STRING_CONST("QOS_AckPolicyVO");
NDIS_STRING STRQoSqueue0OverFlowPolicy          = NDIS_STRING_CONST("QOS_queue0OverFlowPolicy");
NDIS_STRING STRQoSqueue1OverFlowPolicy          = NDIS_STRING_CONST("QOS_queue1OverFlowPolicy");
NDIS_STRING STRQoSqueue2OverFlowPolicy          = NDIS_STRING_CONST("QOS_queue2OverFlowPolicy");
NDIS_STRING STRQoSqueue3OverFlowPolicy          = NDIS_STRING_CONST("QOS_queue3OverFlowPolicy");
/* HP parameters */
NDIS_STRING STR11nEnable                        = NDIS_STRING_CONST("HT_Enable");
NDIS_STRING STRBaPolicyTid_0                    = NDIS_STRING_CONST("BaPolicyTid_0");
NDIS_STRING STRBaPolicyTid_1                    = NDIS_STRING_CONST("BaPolicyTid_1");
NDIS_STRING STRBaPolicyTid_2                    = NDIS_STRING_CONST("BaPolicyTid_2");
NDIS_STRING STRBaPolicyTid_3                    = NDIS_STRING_CONST("BaPolicyTid_3");
NDIS_STRING STRBaPolicyTid_4                    = NDIS_STRING_CONST("BaPolicyTid_4");
NDIS_STRING STRBaPolicyTid_5                    = NDIS_STRING_CONST("BaPolicyTid_5");
NDIS_STRING STRBaPolicyTid_6                    = NDIS_STRING_CONST("BaPolicyTid_6");
NDIS_STRING STRBaPolicyTid_7                    = NDIS_STRING_CONST("BaPolicyTid_7");
NDIS_STRING STRBaInactivityTimeoutTid_0         = NDIS_STRING_CONST("BaInactivityTimeoutTid_0");
NDIS_STRING STRBaInactivityTimeoutTid_1         = NDIS_STRING_CONST("BaInactivityTimeoutTid_1");
NDIS_STRING STRBaInactivityTimeoutTid_2         = NDIS_STRING_CONST("BaInactivityTimeoutTid_2");
NDIS_STRING STRBaInactivityTimeoutTid_3         = NDIS_STRING_CONST("BaInactivityTimeoutTid_3");
NDIS_STRING STRBaInactivityTimeoutTid_4         = NDIS_STRING_CONST("BaInactivityTimeoutTid_4");
NDIS_STRING STRBaInactivityTimeoutTid_5         = NDIS_STRING_CONST("BaInactivityTimeoutTid_5");
NDIS_STRING STRBaInactivityTimeoutTid_6         = NDIS_STRING_CONST("BaInactivityTimeoutTid_6");
NDIS_STRING STRBaInactivityTimeoutTid_7         = NDIS_STRING_CONST("BaInactivityTimeoutTid_7");


/* HW Tx queues mem-blocks allocation thresholds */
NDIS_STRING STRQOStxBlksThresholdBE             = NDIS_STRING_CONST("QOS_txBlksThresholdBE");
NDIS_STRING STRQOStxBlksThresholdBK             = NDIS_STRING_CONST("QOS_txBlksThresholdBK");
NDIS_STRING STRQOStxBlksThresholdVI             = NDIS_STRING_CONST("QOS_txBlksThresholdVI");
NDIS_STRING STRQOStxBlksThresholdVO             = NDIS_STRING_CONST("QOS_txBlksThresholdVO");

/* Traffic Intensity parameters*/
NDIS_STRING STRTrafficIntensityThresHigh        = NDIS_STRING_CONST("TrafficIntensityThresHigh");
NDIS_STRING STRTrafficIntensityThresLow         = NDIS_STRING_CONST("TrafficIntensityThresLow");
NDIS_STRING STRTrafficIntensityTestInterval     = NDIS_STRING_CONST("TrafficIntensityTestInterval");
NDIS_STRING STRTrafficIntensityThresholdEnabled = NDIS_STRING_CONST("TrafficIntensityThresholdEnabled");
NDIS_STRING STRTrafficMonitorMinIntervalPercentage = NDIS_STRING_CONST("TrafficMonitorMinIntervalPercent");


/* Packet Burst parameters */
NDIS_STRING STRQOSPacketBurstEnable             = NDIS_STRING_CONST("QOS_PacketBurstEnable");
NDIS_STRING STRQOSPacketBurstTxOpLimit          = NDIS_STRING_CONST("QOS_PacketBurstTxOpLimit");

/*-----------------------------------*/
/*        QOS classifier Parameters  */
/*-----------------------------------*/
NDIS_STRING STRClsfr_Type                       = NDIS_STRING_CONST("Clsfr_Type");
NDIS_STRING STRNumOfCodePoints                  = NDIS_STRING_CONST("NumOfCodePoints");
NDIS_STRING STRNumOfDstPortClassifiers          = NDIS_STRING_CONST("NumOfDstPortClassifiers");
NDIS_STRING STRNumOfDstIPPortClassifiers        = NDIS_STRING_CONST("NumOfDstIPPortClassifiers");

NDIS_STRING STRDSCPClassifier00_CodePoint       = NDIS_STRING_CONST("DSCPClassifier00_CodePoint");
NDIS_STRING STRDSCPClassifier01_CodePoint       = NDIS_STRING_CONST("DSCPClassifier01_CodePoint");
NDIS_STRING STRDSCPClassifier02_CodePoint       = NDIS_STRING_CONST("DSCPClassifier02_CodePoint");
NDIS_STRING STRDSCPClassifier03_CodePoint       = NDIS_STRING_CONST("DSCPClassifier03_CodePoint");
NDIS_STRING STRDSCPClassifier04_CodePoint       = NDIS_STRING_CONST("DSCPClassifier04_CodePoint");
NDIS_STRING STRDSCPClassifier05_CodePoint       = NDIS_STRING_CONST("DSCPClassifier05_CodePoint");
NDIS_STRING STRDSCPClassifier06_CodePoint       = NDIS_STRING_CONST("DSCPClassifier06_CodePoint");
NDIS_STRING STRDSCPClassifier07_CodePoint       = NDIS_STRING_CONST("DSCPClassifier07_CodePoint");
NDIS_STRING STRDSCPClassifier08_CodePoint       = NDIS_STRING_CONST("DSCPClassifier08_CodePoint");
NDIS_STRING STRDSCPClassifier09_CodePoint       = NDIS_STRING_CONST("DSCPClassifier09_CodePoint");
NDIS_STRING STRDSCPClassifier10_CodePoint       = NDIS_STRING_CONST("DSCPClassifier10_CodePoint");
NDIS_STRING STRDSCPClassifier11_CodePoint       = NDIS_STRING_CONST("DSCPClassifier11_CodePoint");
NDIS_STRING STRDSCPClassifier12_CodePoint       = NDIS_STRING_CONST("DSCPClassifier12_CodePoint");
NDIS_STRING STRDSCPClassifier13_CodePoint       = NDIS_STRING_CONST("DSCPClassifier13_CodePoint");
NDIS_STRING STRDSCPClassifier14_CodePoint       = NDIS_STRING_CONST("DSCPClassifier14_CodePoint");
NDIS_STRING STRDSCPClassifier15_CodePoint       = NDIS_STRING_CONST("DSCPClassifier15_CodePoint");

NDIS_STRING STRDSCPClassifier00_DTag        = NDIS_STRING_CONST("DSCPClassifier00_DTag");
NDIS_STRING STRDSCPClassifier01_DTag        = NDIS_STRING_CONST("DSCPClassifier01_DTag");
NDIS_STRING STRDSCPClassifier02_DTag        = NDIS_STRING_CONST("DSCPClassifier02_DTag");
NDIS_STRING STRDSCPClassifier03_DTag        = NDIS_STRING_CONST("DSCPClassifier03_DTag");
NDIS_STRING STRDSCPClassifier04_DTag        = NDIS_STRING_CONST("DSCPClassifier04_DTag");
NDIS_STRING STRDSCPClassifier05_DTag        = NDIS_STRING_CONST("DSCPClassifier05_DTag");
NDIS_STRING STRDSCPClassifier06_DTag        = NDIS_STRING_CONST("DSCPClassifier06_DTag");
NDIS_STRING STRDSCPClassifier07_DTag        = NDIS_STRING_CONST("DSCPClassifier07_DTag");
NDIS_STRING STRDSCPClassifier08_DTag        = NDIS_STRING_CONST("DSCPClassifier08_DTag");
NDIS_STRING STRDSCPClassifier09_DTag        = NDIS_STRING_CONST("DSCPClassifier09_DTag");
NDIS_STRING STRDSCPClassifier10_DTag        = NDIS_STRING_CONST("DSCPClassifier10_DTag");
NDIS_STRING STRDSCPClassifier11_DTag        = NDIS_STRING_CONST("DSCPClassifier11_DTag");
NDIS_STRING STRDSCPClassifier12_DTag        = NDIS_STRING_CONST("DSCPClassifier12_DTag");
NDIS_STRING STRDSCPClassifier13_DTag        = NDIS_STRING_CONST("DSCPClassifier13_DTag");
NDIS_STRING STRDSCPClassifier14_DTag        = NDIS_STRING_CONST("DSCPClassifier14_DTag");
NDIS_STRING STRDSCPClassifier15_DTag        = NDIS_STRING_CONST("DSCPClassifier15_DTag");


NDIS_STRING STRPortClassifier00_Port            = NDIS_STRING_CONST("PortClassifier00_Port");
NDIS_STRING STRPortClassifier01_Port            = NDIS_STRING_CONST("PortClassifier01_Port");
NDIS_STRING STRPortClassifier02_Port            = NDIS_STRING_CONST("PortClassifier02_Port");
NDIS_STRING STRPortClassifier03_Port            = NDIS_STRING_CONST("PortClassifier03_Port");
NDIS_STRING STRPortClassifier04_Port            = NDIS_STRING_CONST("PortClassifier04_Port");
NDIS_STRING STRPortClassifier05_Port            = NDIS_STRING_CONST("PortClassifier05_Port");
NDIS_STRING STRPortClassifier06_Port            = NDIS_STRING_CONST("PortClassifier06_Port");
NDIS_STRING STRPortClassifier07_Port            = NDIS_STRING_CONST("PortClassifier07_Port");
NDIS_STRING STRPortClassifier08_Port            = NDIS_STRING_CONST("PortClassifier08_Port");
NDIS_STRING STRPortClassifier09_Port            = NDIS_STRING_CONST("PortClassifier09_Port");
NDIS_STRING STRPortClassifier10_Port            = NDIS_STRING_CONST("PortClassifier10_Port");
NDIS_STRING STRPortClassifier11_Port            = NDIS_STRING_CONST("PortClassifier11_Port");
NDIS_STRING STRPortClassifier12_Port            = NDIS_STRING_CONST("PortClassifier12_Port");
NDIS_STRING STRPortClassifier13_Port            = NDIS_STRING_CONST("PortClassifier13_Port");
NDIS_STRING STRPortClassifier14_Port            = NDIS_STRING_CONST("PortClassifier14_Port");
NDIS_STRING STRPortClassifier15_Port            = NDIS_STRING_CONST("PortClassifier15_Port");

NDIS_STRING STRPortClassifier00_DTag            = NDIS_STRING_CONST("PortClassifier00_DTag");
NDIS_STRING STRPortClassifier01_DTag            = NDIS_STRING_CONST("PortClassifier01_DTag");
NDIS_STRING STRPortClassifier02_DTag            = NDIS_STRING_CONST("PortClassifier02_DTag");
NDIS_STRING STRPortClassifier03_DTag            = NDIS_STRING_CONST("PortClassifier03_DTag");
NDIS_STRING STRPortClassifier04_DTag            = NDIS_STRING_CONST("PortClassifier04_DTag");
NDIS_STRING STRPortClassifier05_DTag            = NDIS_STRING_CONST("PortClassifier05_DTag");
NDIS_STRING STRPortClassifier06_DTag            = NDIS_STRING_CONST("PortClassifier06_DTag");
NDIS_STRING STRPortClassifier07_DTag            = NDIS_STRING_CONST("PortClassifier07_DTag");
NDIS_STRING STRPortClassifier08_DTag            = NDIS_STRING_CONST("PortClassifier08_DTag");
NDIS_STRING STRPortClassifier09_DTag            = NDIS_STRING_CONST("PortClassifier09_DTag");
NDIS_STRING STRPortClassifier10_DTag            = NDIS_STRING_CONST("PortClassifier10_DTag");
NDIS_STRING STRPortClassifier11_DTag            = NDIS_STRING_CONST("PortClassifier11_DTag");
NDIS_STRING STRPortClassifier12_DTag            = NDIS_STRING_CONST("PortClassifier12_DTag");
NDIS_STRING STRPortClassifier13_DTag            = NDIS_STRING_CONST("PortClassifier13_DTag");
NDIS_STRING STRPortClassifier14_DTag            = NDIS_STRING_CONST("PortClassifier14_DTag");
NDIS_STRING STRPortClassifier15_DTag            = NDIS_STRING_CONST("PortClassifier15_DTag");

NDIS_STRING STRIPPortClassifier00_IPAddress     = NDIS_STRING_CONST("IPPortClassifier00_IPAddress");
NDIS_STRING STRIPPortClassifier01_IPAddress     = NDIS_STRING_CONST("IPPortClassifier01_IPAddress");
NDIS_STRING STRIPPortClassifier02_IPAddress     = NDIS_STRING_CONST("IPPortClassifier02_IPAddress");
NDIS_STRING STRIPPortClassifier03_IPAddress     = NDIS_STRING_CONST("IPPortClassifier03_IPAddress");
NDIS_STRING STRIPPortClassifier04_IPAddress     = NDIS_STRING_CONST("IPPortClassifier04_IPAddress");
NDIS_STRING STRIPPortClassifier05_IPAddress     = NDIS_STRING_CONST("IPPortClassifier05_IPAddress");
NDIS_STRING STRIPPortClassifier06_IPAddress     = NDIS_STRING_CONST("IPPortClassifier06_IPAddress");
NDIS_STRING STRIPPortClassifier07_IPAddress     = NDIS_STRING_CONST("IPPortClassifier07_IPAddress");
NDIS_STRING STRIPPortClassifier08_IPAddress     = NDIS_STRING_CONST("IPPortClassifier08_IPAddress");
NDIS_STRING STRIPPortClassifier09_IPAddress     = NDIS_STRING_CONST("IPPortClassifier09_IPAddress");
NDIS_STRING STRIPPortClassifier10_IPAddress     = NDIS_STRING_CONST("IPPortClassifier10_IPAddress");
NDIS_STRING STRIPPortClassifier11_IPAddress     = NDIS_STRING_CONST("IPPortClassifier11_IPAddress");
NDIS_STRING STRIPPortClassifier12_IPAddress     = NDIS_STRING_CONST("IPPortClassifier12_IPAddress");
NDIS_STRING STRIPPortClassifier13_IPAddress     = NDIS_STRING_CONST("IPPortClassifier13_IPAddress");
NDIS_STRING STRIPPortClassifier14_IPAddress     = NDIS_STRING_CONST("IPPortClassifier14_IPAddress");
NDIS_STRING STRIPPortClassifier15_IPAddress     = NDIS_STRING_CONST("IPPortClassifier15_IPAddress");

NDIS_STRING STRIPPortClassifier00_Port          = NDIS_STRING_CONST("IPPortClassifier00_Port");
NDIS_STRING STRIPPortClassifier01_Port          = NDIS_STRING_CONST("IPPortClassifier01_Port");
NDIS_STRING STRIPPortClassifier02_Port          = NDIS_STRING_CONST("IPPortClassifier02_Port");
NDIS_STRING STRIPPortClassifier03_Port          = NDIS_STRING_CONST("IPPortClassifier03_Port");
NDIS_STRING STRIPPortClassifier04_Port          = NDIS_STRING_CONST("IPPortClassifier04_Port");
NDIS_STRING STRIPPortClassifier05_Port          = NDIS_STRING_CONST("IPPortClassifier05_Port");
NDIS_STRING STRIPPortClassifier06_Port          = NDIS_STRING_CONST("IPPortClassifier06_Port");
NDIS_STRING STRIPPortClassifier07_Port          = NDIS_STRING_CONST("IPPortClassifier07_Port");
NDIS_STRING STRIPPortClassifier08_Port          = NDIS_STRING_CONST("IPPortClassifier08_Port");
NDIS_STRING STRIPPortClassifier09_Port          = NDIS_STRING_CONST("IPPortClassifier09_Port");
NDIS_STRING STRIPPortClassifier10_Port          = NDIS_STRING_CONST("IPPortClassifier10_Port");
NDIS_STRING STRIPPortClassifier11_Port          = NDIS_STRING_CONST("IPPortClassifier11_Port");
NDIS_STRING STRIPPortClassifier12_Port          = NDIS_STRING_CONST("IPPortClassifier12_Port");
NDIS_STRING STRIPPortClassifier13_Port          = NDIS_STRING_CONST("IPPortClassifier13_Port");
NDIS_STRING STRIPPortClassifier14_Port          = NDIS_STRING_CONST("IPPortClassifier14_Port");
NDIS_STRING STRIPPortClassifier15_Port          = NDIS_STRING_CONST("IPPortClassifier15_Port");

NDIS_STRING STRIPPortClassifier00_DTag          = NDIS_STRING_CONST("IPPortClassifier00_DTag");
NDIS_STRING STRIPPortClassifier01_DTag          = NDIS_STRING_CONST("IPPortClassifier01_DTag");
NDIS_STRING STRIPPortClassifier02_DTag          = NDIS_STRING_CONST("IPPortClassifier02_DTag");
NDIS_STRING STRIPPortClassifier03_DTag          = NDIS_STRING_CONST("IPPortClassifier03_DTag");
NDIS_STRING STRIPPortClassifier04_DTag          = NDIS_STRING_CONST("IPPortClassifier04_DTag");
NDIS_STRING STRIPPortClassifier05_DTag          = NDIS_STRING_CONST("IPPortClassifier05_DTag");
NDIS_STRING STRIPPortClassifier06_DTag          = NDIS_STRING_CONST("IPPortClassifier06_DTag");
NDIS_STRING STRIPPortClassifier07_DTag          = NDIS_STRING_CONST("IPPortClassifier07_DTag");
NDIS_STRING STRIPPortClassifier08_DTag          = NDIS_STRING_CONST("IPPortClassifier08_DTag");
NDIS_STRING STRIPPortClassifier09_DTag          = NDIS_STRING_CONST("IPPortClassifier09_DTag");
NDIS_STRING STRIPPortClassifier10_DTag          = NDIS_STRING_CONST("IPPortClassifier10_DTag");
NDIS_STRING STRIPPortClassifier11_DTag          = NDIS_STRING_CONST("IPPortClassifier11_DTag");
NDIS_STRING STRIPPortClassifier12_DTag          = NDIS_STRING_CONST("IPPortClassifier12_DTag");
NDIS_STRING STRIPPortClassifier13_DTag          = NDIS_STRING_CONST("IPPortClassifier13_DTag");
NDIS_STRING STRIPPortClassifier14_DTag          = NDIS_STRING_CONST("IPPortClassifier14_DTag");
NDIS_STRING STRIPPortClassifier15_DTag          = NDIS_STRING_CONST("IPPortClassifier15_DTag");

/*-----------------------------
   Rx Data Filter parameters
-----------------------------*/
NDIS_STRING STRRxDataFiltersEnabled             = NDIS_STRING_CONST("RxDataFilters_Enabled");
NDIS_STRING STRRxDataFiltersDefaultAction       = NDIS_STRING_CONST("RxDataFilters_DefaultAction");

NDIS_STRING STRRxDataFiltersFilter1Offset       = NDIS_STRING_CONST("RxDataFilters_Filter1Offset");
NDIS_STRING STRRxDataFiltersFilter1Mask         = NDIS_STRING_CONST("RxDataFilters_Filter1Mask");
NDIS_STRING STRRxDataFiltersFilter1Pattern      = NDIS_STRING_CONST("RxDataFilters_Filter1Pattern");

NDIS_STRING STRRxDataFiltersFilter2Offset       = NDIS_STRING_CONST("RxDataFilters_Filter2Offset");
NDIS_STRING STRRxDataFiltersFilter2Mask         = NDIS_STRING_CONST("RxDataFilters_Filter2Mask");
NDIS_STRING STRRxDataFiltersFilter2Pattern      = NDIS_STRING_CONST("RxDataFilters_Filter2Pattern");

NDIS_STRING STRRxDataFiltersFilter3Offset       = NDIS_STRING_CONST("RxDataFilters_Filter3Offset");
NDIS_STRING STRRxDataFiltersFilter3Mask         = NDIS_STRING_CONST("RxDataFilters_Filter3Mask");
NDIS_STRING STRRxDataFiltersFilter3Pattern      = NDIS_STRING_CONST("RxDataFilters_Filter3Pattern");

NDIS_STRING STRRxDataFiltersFilter4Offset       = NDIS_STRING_CONST("RxDataFilters_Filter4Offset");
NDIS_STRING STRRxDataFiltersFilter4Mask         = NDIS_STRING_CONST("RxDataFilters_Filter4Mask");
NDIS_STRING STRRxDataFiltersFilter4Pattern      = NDIS_STRING_CONST("RxDataFilters_Filter4Pattern");


/*---------------------------
    Measurement parameters
-----------------------------*/
NDIS_STRING STRMeasurTrafficThreshold           = NDIS_STRING_CONST( "MeasurTrafficThreshold" );
NDIS_STRING STRMeasurMaxDurationOnNonServingChannel = NDIS_STRING_CONST( "MeasurMaxDurationOnNonServingChannel" );

/*---------------------------
      XCC Manager parameters
-----------------------------*/
#ifdef XCC_MODULE_INCLUDED
NDIS_STRING STRXCCModeEnabled                   = NDIS_STRING_CONST( "XCCModeEnabled" );
#endif

NDIS_STRING STRXCCTestIgnoreDeAuth0             = NDIS_STRING_CONST( "XCCTestRogeAP" );

/*-----------------------------------*/
/*   EEPROM-less support             */
/*-----------------------------------*/
NDIS_STRING STREEPROMlessModeSupported          = NDIS_STRING_CONST( "EEPROMlessModeSupported" );
NDIS_STRING STRstationMacAddress                = NDIS_STRING_CONST("dot11StationID");


/*-----------------------------------*/
/*   INI file to configuration       */
/*-----------------------------------*/

NDIS_STRING SendINIBufferToUser                 = NDIS_STRING_CONST("SendINIBufferToUserMode");

/*-------------------------------------------
   RSSI/SNR Weights for Average calculations   
--------------------------------------------*/

NDIS_STRING STRRssiBeaconAverageWeight = NDIS_STRING_CONST("RssiBeaconAverageWeight");
NDIS_STRING STRRssiPacketAverageWeight = NDIS_STRING_CONST("RssiPacketAverageWeight");
NDIS_STRING STRSnrBeaconAverageWeight  = NDIS_STRING_CONST("SnrBeaconAverageWeight");
NDIS_STRING STRSnrPacketAverageWeight  = NDIS_STRING_CONST("SnrPacketAverageWeight");

/*-----------------------------------*/
/*   Scan concentrator parameters    */
/*-----------------------------------*/
NDIS_STRING STRMinimumDurationBetweenOsScans     = NDIS_STRING_CONST( "MinimumDurationBetweenOsScans" );
NDIS_STRING STRDfsPassiveDwellTimeMs             = NDIS_STRING_CONST( "DfsPassiveDwellTimeMs" );
NDIS_STRING STRScanPushMode                      = NDIS_STRING_CONST( "ScanPushMode" );

NDIS_STRING STRParseWSCInBeacons      = NDIS_STRING_CONST( "ParseWSCInBeacons" );

/*-----------------------------------*/
/*      Current BSS parameters       */
/*-----------------------------------*/
NDIS_STRING STRNullDataKeepAliveDefaultPeriod  = NDIS_STRING_CONST("NullDataKeepAliveDefaultPeriod");

/*-----------------------------------*/
/*      Context-Engine parameters    */
/*-----------------------------------*/
NDIS_STRING STRContextSwitchRequired  = NDIS_STRING_CONST("ContextSwitchRequired");

/*-----------------------------------*/
/*      Radio parameters             */
/*-----------------------------------*/
NDIS_STRING STRTXBiPReferencePDvoltage_2_4G =       NDIS_STRING_CONST("TXBiPReferencePDvoltage");
NDIS_STRING STRTxBiPReferencePower_2_4G =           NDIS_STRING_CONST("TxBiPReferencePower");  
NDIS_STRING STRTxBiPOffsetdB_2_4G =                 NDIS_STRING_CONST("TxBiPOffsetdB_2_4G");  
NDIS_STRING STRTxPerRatePowerLimits_2_4G_Normal =  	NDIS_STRING_CONST("TxPerRatePowerLimits_2_4G_Normal");
NDIS_STRING STRTxPerRatePowerLimits_2_4G_Degraded = NDIS_STRING_CONST("TxPerRatePowerLimits_2_4G_Degraded");
NDIS_STRING STRTxPerChannelPowerLimits_2_4G_11b =   NDIS_STRING_CONST("TxPerChannelPowerLimits_2_4G_11b");
NDIS_STRING STRTxPerChannelPowerLimits_2_4G_OFDM =  NDIS_STRING_CONST("TxPerChannelPowerLimits_2_4G_OFDM");
NDIS_STRING STRTxPDVsRateOffsets_2_4G =             NDIS_STRING_CONST("TxPDVsRateOffsets_2_4G");
NDIS_STRING STRTxIbiasTable_2_4G =          	  	NDIS_STRING_CONST("TxIbiasTable_2_4G");
NDIS_STRING STRRxFemInsertionLoss_2_4G =            NDIS_STRING_CONST("RxFemInsertionLoss_2_4G");
NDIS_STRING STRRxTraceInsertionLoss_2_4G =          NDIS_STRING_CONST("RxTraceInsertionLoss_2_4G");
NDIS_STRING STRTXTraceLoss_2_4G =                   NDIS_STRING_CONST("TXTraceLoss_2_4G");
NDIS_STRING STRRxRssiAndProcessCompensation_2_4G =  NDIS_STRING_CONST("RxRssiAndProcessCompensation_2_4G");
NDIS_STRING STRTXBiPReferencePDvoltage_5G =         NDIS_STRING_CONST("TXBiPReferencePDvoltage_5G");
NDIS_STRING STRTxBiPReferencePower_5G =             NDIS_STRING_CONST("TxBiPReferencePower_5G");
NDIS_STRING STRTxBiPOffsetdB_5G =                   NDIS_STRING_CONST("TxBiPOffsetdB_5G");
NDIS_STRING STRTxPerRatePowerLimits_5G_Normal =     NDIS_STRING_CONST("TxPerRatePowerLimits_5G_Normal");
NDIS_STRING STRTxPerRatePowerLimits_5G_Degraded =   NDIS_STRING_CONST("TxPerRatePowerLimits_5G_Degraded");
NDIS_STRING STRTxPerChannelPowerLimits_5G_OFDM =    NDIS_STRING_CONST("TxPerChannelPowerLimits_5G_OFDM");
NDIS_STRING STRTxPDVsRateOffsets_5G =               NDIS_STRING_CONST("TxPDVsRateOffsets_5G");
NDIS_STRING STRTxIbiasTable_5G =                    NDIS_STRING_CONST("TxIbiasTable_5G");
NDIS_STRING STRRxFemInsertionLoss_5G =              NDIS_STRING_CONST("RxFemInsertionLoss_5G");
NDIS_STRING STRRxTraceInsertionLoss_5G =            NDIS_STRING_CONST("RxTraceInsertionLoss_5G");
NDIS_STRING STRTXTraceLoss_5G =                     NDIS_STRING_CONST("TXTraceLoss_5G");
NDIS_STRING STRRxRssiAndProcessCompensation_5G =    NDIS_STRING_CONST("RxRssiAndProcessCompensation_5G");
NDIS_STRING STRFRefClock =    						NDIS_STRING_CONST("STRFRefClock");
NDIS_STRING STRFRefClockSettingTime =    			NDIS_STRING_CONST("STRFRefClockSettingTime");
NDIS_STRING STRFRefClockStatus =    				NDIS_STRING_CONST("FRefClockStatus");
NDIS_STRING STRTXBiPFEMAutoDetect =    				NDIS_STRING_CONST("TXBiPFEMAutoDetect");
NDIS_STRING STRTXBiPFEMManufacturer =    			NDIS_STRING_CONST("TXBiPFEMManufacturer");
NDIS_STRING STRClockValidOnWakeup =    	    		NDIS_STRING_CONST("ClockValidOnWakeup");
NDIS_STRING STRDC2DCMode =    	    	            NDIS_STRING_CONST("DC2DCMode");
NDIS_STRING STRSingle_Dual_Band_Solution =          NDIS_STRING_CONST("Single_Dual_Band_Solution");


/*-----------------------------------*/
/*      Driver-Main parameters       */
/*-----------------------------------*/
NDIS_STRING STRWlanDrvThreadPriority = NDIS_STRING_CONST("WlanDrvThreadPriority");
NDIS_STRING STRBusDrvThreadPriority  = NDIS_STRING_CONST("BusDrvThreadPriority");
NDIS_STRING STRSdioBlkSizeShift      = NDIS_STRING_CONST("SdioBlkSizeShift");



/*
NDIS_STRING STRPctACXRxMemoryPool       = NDIS_STRING_CONST( "PctACXRxMemoryPool" );
NDIS_STRING STRSendPacketsPerOID        = NDIS_STRING_CONST( "Dot11SendPacketsPerOID" );
NDIS_STRING STRFragCacheSize            = NDIS_STRING_CONST( "FragCacheSize" );
*/

static void regConvertStringtoMACAddress(TI_UINT8 *staMACAddressString,TI_UINT8 *MacAddressArray);
static void regConvertStringtoBeaconIETable(TI_UINT8 *staIpAddressString,TI_UINT8 *BeaconIEArray, TI_UINT8 size);
static void regConvertStringtoCoexActivityTable(TI_UINT8 *strCoexActivityTable, TI_UINT32 numOfElements, TCoexActivity *CoexActivityArray, TI_UINT8 strSize);

static int decryptWEP( TI_INT8* pSrc, TI_INT8* pDst, TI_UINT32 len);
static short _btoi ( char *sptr, short slen, int *pi, short base );
static void initValusFromRgstryString(  TI_INT8* pSrc,  TI_INT8* pDst,  TI_UINT32 len);



static void readRates(TWlanDrvIfObjPtr pAdapter, TInitTable *pInitTable);
static void decryptScanControlTable(TI_UINT8* src, TI_UINT8* dst, USHORT len);

static TI_UINT32 regReadIntegerTable(TWlanDrvIfObjPtr   pAdapter,
                                PNDIS_STRING        pParameterName,
                                TI_INT8*               pDefaultValue,
                                USHORT              defaultLen,
                                TI_UINT8*              pUnsignedParameter,
                                TI_INT8*               pSignedParameter,
                                TI_UINT32*             pEntriesNumber,
                                TI_UINT8               uParameterSize,
                                TI_BOOL                bHex);

static void assignRegValue(TI_UINT32* lValue, PNDIS_CONFIGURATION_PARAMETER ndisParameter);

static void parse_filter_request(TRxDataFilterRequest* request, TI_UINT8 offset, char * mask, TI_UINT8 maskLength, char * pattern, TI_UINT8 patternLength);

static void regReadIntegerParameter (
                 TWlanDrvIfObjPtr       pAdapter,
                 PNDIS_STRING           pParameterName,
                 TI_UINT32              defaultValue,
                 TI_UINT32              minValue,
                 TI_UINT32              maxValue,
                 TI_UINT8               parameterSize,
                 TI_UINT8*              pParameter);

static void regReadIntegerParameterHex (
                 TWlanDrvIfObjPtr       pAdapter,
                 PNDIS_STRING           pParameterName,
                 TI_UINT32              defaultValue,
                 TI_UINT32              minValue,
                 TI_UINT32              maxValue,
                 TI_UINT8               defaultSize,
                 TI_UINT8 *             pParameter);

static void regReadStringParameter (
                 TWlanDrvIfObjPtr       pAdapter,
                 PNDIS_STRING           pParameterName,
                 TI_INT8*               pDefaultValue,
                 USHORT                 defaultLen,
                 TI_UINT8*              pParameter,
                 void*                  pParameterSize);

static void regReadWepKeyParameter (TWlanDrvIfObjPtr pAdapter, TI_UINT8 *pKeysStructure, TI_UINT8 defaultKeyId);


/* ---------------------------------------------------------------------------*/
/* Converts a string to a signed int. Assumes base 10. Assumes positive*/
/*    number*/
/**/
/* Returns value on success, -1 on failure*/
/**/
/* ---------------------------------------------------------------------------*/
static TI_UINT32 tiwlnstrtoi (char *num, TI_UINT32 length)
{
  TI_UINT32 value;

  if(num == NULL || length == 0 )
  {
    return 0;
  }

  for(value=0;length&&*num;num++,length--)
  {
    if(*num<='9'&&*num>= '0')
    {
      value=(value*10)+(*num - '0');
    }
    else { /* Out of range*/
      break;
    }
  }
  return value;
}

/* ---------------------------------------------------------------------------*/
/* Converts a string to a signed int. Assumes base 16. Assumes positive*/
/*    number*/
/**/
/* Returns value on success, -1 on failure*/
/**/
/* ---------------------------------------------------------------------------*/
static TI_UINT32 tiwlnstrtoi_hex (TI_UINT8 *num, TI_UINT32 length)
{
  TI_UINT32 value = 0;

  if (num == NULL || length == 0)
  {
      return 0;
  }

  for (value = 0; length && *num; num++, length--)
  {
    if (*num <= '9' && *num >= '0')
    {
      value = (value * 16) + (*num - '0');
    }
    else if (*num <= 'f' && *num >= 'a')
    {
        value = (value * 16) + (*num - (TI_UINT8)'a' + 10);
    }
    else if (*num <= 'F' && *num >= 'A')
    {
        value = (value * 16) + (*num - (TI_UINT8)'A' + 10);
    }
    else
    { /* Out of range*/
        break;
    }
  }
  
  return value;
}

/*-----------------------------------------------------------------------------

Routine Name:

    regConvertStringtoMACAddress

Routine Description: Converts the MAC Adrress in a form of string readen from the Registry 
to the MAC Address Array to be stored in the init_table struct 


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
static void regConvertStringtoMACAddress(TI_UINT8 *staMACAddressString,TI_UINT8 *MacAddressArray)
{

char *ptr;
TI_UINT8 *tmpMacAddr;
TI_UINT8 value=0,value_l,value_h,add_value;
int i;


    /* Take the pointer to the string MAC Address to convert it to the Array MAC Address */
    ptr=(char *)staMACAddressString;
    tmpMacAddr = MacAddressArray;

    for(i=0 ; i<MAC_ADDR_LEN ; ptr++)
    {
        
        /* The value can be or "0-9" or from "a-f" */
        value_l = (*ptr-'0');
        value_h = (*ptr - 'a');

        /*PRINTF(DBG_REGISTRY,("value_l [%d] value_h [%d] *ptr %c value %d\n",value_l,value_h,*ptr,value));*/

        if( (value_l <= 9) || (value_h <= 15 ) )
        {
            /* We are in an expected range */
            /* nCheck if 0-9 */
            if(value_l <= 9 )
            {
                add_value = value_l;
            }
            /* Check if a-f */
            else
            {
                /* 'a' is in fact 10 decimal in hexa */
                add_value = value_h + 10;
            }
            value = value*16 + add_value;               
        /*  PRINTF(DBG_REGISTRY,("value %d add_value %d  \n",value,add_value));*/


        }
        else
        {
            tmpMacAddr[i] = value;
        /*  PRINTF(DBG_REGISTRY,("tmpMacAddr[%d]  is %x\n",i,tmpMacAddr[i]));*/
            value = 0;
            i++;
        }

    }
}



/*-----------------------------------------------------------------------------

Routine Name:

    regFillInitTable

Routine Description:


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
void
regFillInitTable(
                TWlanDrvIfObjPtr pAdapter,
                void* pInitTable
                )
{
    TI_UINT8 bssidBroadcast[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    char     dummySsidString[MAX_SSID_LEN];

    TI_UINT8 *ClsfrIp = "0a 03 01 c9";
    TI_UINT8 ClsfrIpString[16];
    TI_UINT8 ClsfrIpStringSize;

  /* EEPROM-less : MAC address */
    TI_UINT8 regMACstrLen = REG_MAC_ADDR_STR_LEN;
    TI_UINT8 staMACAddress[REG_MAC_ADDR_STR_LEN];
    TI_UINT8 defStaMacAddress0[]= "00 22 11 33 44 55";
    TI_UINT8 defStaMacAddress1[]= "11 11 12 13 14 15";
    TI_UINT8 defStaMacAddress2[]= "12 21 22 23 24 25";
    TI_UINT8 defStaMacAddress3[]= "13 31 32 33 34 35";
    TI_UINT8 defStaMacAddress4[]= "14 41 42 43 44 45";
    TI_UINT8 defStaMacAddress5[]= "15 51 52 53 54 55";
    TI_UINT8 defStaMacAddress6[]= "16 61 62 63 64 65";
    TI_UINT8 defStaMacAddress7[]= "17 71 72 73 74 75";


    TI_UINT8 regArpIpStrLen = REG_ARP_IP_ADDR_STR_LEN ; 
    TI_UINT8 staArpIpAddress[REG_ARP_IP_ADDR_STR_LEN];
    TI_UINT8 defArpIpAddress[] =  "0a 00 00 0a" ;       /*value by default*/

    /*defaults values for beacon IE table*/
    /*TI_UINT8 defBeaconIETableSize = 0 ;*/
    TI_UINT8 defBeaconIETable[] = "00 01 01 01 32 01 2a 01 03 01 06 01 07 01 20 01 25 01 23 01 30 01 28 01 2e 01 3d 01 85 01 dd 01 00 52 f2 02 00 01";
    /*TI_UINT8 tmpIeTable[BEACON_FILTER_TABLE_MAX_SIZE] ;*/
    TI_UINT8 staBeaconFilterIETable[BEACON_FILTER_STRING_MAX_LEN] ;
    TI_UINT8 tmpIeTableSize = 37;
    TI_UINT8 strSize = 113;

    TI_UINT8 defRxRssiAndProcessCompensation_2_4G[] = "ec,f6,00,0c,18,f8,fc,00,08,10,f0,f8,00,0a,14";   
    TI_UINT8 tmpRssiTableSize = RSSI_AND_PROCESS_COMPENSATION_TABLE_SIZE;
    TI_UINT8 staRssiAndProcessCompensation[RSSI_AND_PROCESS_COMPENSATION_TABLE_SIZE] ;
    TI_UINT8 RssiSize = 113;


    /* defaults values for CoexActivity table*/
    /* example: WLAN(0), BT_VOICE(0), defPrio(20), raisePrio(25), minServ(0), maxServ(1ms) */
    TI_UINT8 defCoexActivityTable[] = ""; /* Sample "01 00 14 19 0000 0001 " */
    TI_UINT8 strCoexActivityTable[COEX_ACTIVITY_TABLE_MAX_NUM*COEX_ACTIVITY_TABLE_SIZE] ;
    TI_UINT8 strCoexActivitySize = 0;

    TI_UINT32 filterOffset = 0;
    char filterMask[16];
    TI_UINT8 filterMaskLength;
    char filterPattern[16];
    TI_UINT8 filterPatternLength;

    TInitTable* p = (TInitTable*) pInitTable;
    USHORT  tableLen = 0;
    USHORT  loopIndex = 0;
    TI_UINT8   ScanControlTable24Tmp[2 * NUM_OF_CHANNELS_24];
    TI_UINT8   ScanControlTable5Tmp[2 * NUM_OF_CHANNELS_5];
    TI_UINT8   ScanControlTable24Def[2* NUM_OF_CHANNELS_24] = "FFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    TI_UINT8   ScanControlTable5Def[2 * NUM_OF_CHANNELS_5] = "FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF0000000000000000000000000000000000000000000000000000000000000000000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF000000FF0000000000000000FF000000FF000000FF000000FF000000FF000000000000000000000000000000";
    TI_UINT8   reportSeverityTableDefaults[REPORT_SEVERITY_MAX] = "0001101";
    TI_UINT8   reportModuleTableDefaults[REPORT_FILES_NUM];
    TI_UINT16  reportSeverityTableLen;
    TI_UINT16  reportModuleTableLen;

    TI_UINT8   uSmeTempList[ 255 ];
    TI_UINT32  uSmeScanIntervalsTempList[ 255 ];
    TI_UINT32  uTempEntriesCount, uSmeGChannelsCount, uIndex;
    TI_UINT32  uWiFiMode = 0;

    int macIndex ; /*used for group address filtering*/

    PRINT(DBG_REGISTRY_LOUD, "TIWL: Reading InitTable parameters\n");
    
    /*set all report modules.as default*/
    memset(reportModuleTableDefaults, '1', REPORT_FILES_NUM );
    
    /* Reset structure */
    NdisZeroMemory(p, sizeof(TInitTable));
    
    /*reads the arp ip from table*/
    regReadStringParameter(pAdapter ,&STRArp_Ip_Addr,
                            (TI_INT8*)(defArpIpAddress),REG_ARP_IP_ADDR_STR_LEN,
                            (TI_UINT8*)staArpIpAddress,&regArpIpStrLen ) ;

    initValusFromRgstryString( (TI_INT8*)(staArpIpAddress), (TI_INT8*)&p->twdInitParams.tArpIpFilter.addr, 4);


    regReadIntegerParameter(pAdapter, &STRArp_Ip_Filter_Ena,
                            DEF_FILTER_ENABLE_VALUE, MIN_FILTER_ENABLE_VALUE, MAX_FILTER_ENABLE_VALUE,
                            sizeof p->twdInitParams.tArpIpFilter.isFilterEnabled, 
                            (TI_UINT8*)&p->twdInitParams.tArpIpFilter.isFilterEnabled );

  
      
    /* Beacon filter*/
    /*is the desired state ENABLED ?*/
    regReadIntegerParameter(pAdapter, &STRBeaconFilterDesiredState,
                            DEF_BEACON_FILTER_ENABLE_VALUE, MIN_BEACON_FILTER_ENABLE_VALUE, MAX_BEACON_FILTER_ENABLE_VALUE,
                            sizeof p->siteMgrInitParams.beaconFilterParams.desiredState, 
                            (TI_UINT8*)&p->siteMgrInitParams.beaconFilterParams.desiredState );
    
    regReadIntegerParameter(pAdapter, &STRBeaconFilterStored,
                            DEF_NUM_STORED_FILTERS, MIN_NUM_STORED_FILTERS, MAX_NUM_STORED_FILTERS,
                            sizeof p->siteMgrInitParams.beaconFilterParams.numOfStored, 
                            (TI_UINT8*)&p->siteMgrInitParams.beaconFilterParams.numOfStored );

    /*Read the beacon filter IE table*/
    /*Read the size of the table*/
    regReadIntegerParameter(pAdapter, &STRBeaconIETableSize,
                            BEACON_FILTER_IE_TABLE_DEF_SIZE, BEACON_FILTER_IE_TABLE_MIN_SIZE,
                            BEACON_FILTER_IE_TABLE_MAX_SIZE,
                            sizeof p->siteMgrInitParams.beaconFilterParams.IETableSize, 
                            (TI_UINT8*)(&p->siteMgrInitParams.beaconFilterParams.IETableSize) );
    
    tmpIeTableSize = p->siteMgrInitParams.beaconFilterParams.IETableSize;
    
    /*Read the number of elements in the table ( this is because 221 has 5 values following it )*/
    regReadIntegerParameter(pAdapter, &STRBeaconIETableNumOfElem,
                            DEF_BEACON_FILTER_IE_TABLE_NUM, BEACON_FILTER_IE_TABLE_MIN_NUM,
                            BEACON_FILTER_IE_TABLE_MAX_NUM,
                            sizeof p->siteMgrInitParams.beaconFilterParams.numOfElements, 
                            (TI_UINT8*)(&p->siteMgrInitParams.beaconFilterParams.numOfElements) );

    /*printk("\n  OsRgstr tmpIeTableSize = %d numOfElems = %d" , tmpIeTableSize , p->siteMgrInitParams.beaconFilterParams.numOfElements) ;*/
    strSize = tmpIeTableSize*2 +tmpIeTableSize - 1 ; /*includes spaces between bytes*/
    if ( ( tmpIeTableSize  > 0 ) && ( tmpIeTableSize <= BEACON_FILTER_IE_TABLE_MAX_SIZE) )
    {
    
        regReadStringParameter(pAdapter, &STRBeaconIETable ,
                            (TI_INT8*)(defBeaconIETable), strSize,
                            (TI_UINT8*)staBeaconFilterIETable, &strSize);

        regConvertStringtoBeaconIETable(staBeaconFilterIETable , (TI_UINT8*)&p->siteMgrInitParams.beaconFilterParams.IETable[0]/*(TI_UINT8*)&(tmpIeTable[0] )*/ , tmpIeTableSize);
    }

    /* MAC ADDRESSES FILTER*/
    regReadIntegerParameter(pAdapter, &STRFilterEnabled,
                            DEF_FILTER_ENABLE_VALUE, MIN_FILTER_ENABLE_VALUE,
                            MAX_FILTER_ENABLE_VALUE,
                            sizeof p->twdInitParams.tMacAddrFilter.isFilterEnabled, 
                            (TI_UINT8*) &p->twdInitParams.tMacAddrFilter.isFilterEnabled);

    regReadIntegerParameter(pAdapter, &STRnumGroupAddrs,
                            NUM_GROUP_ADDRESS_VALUE_DEF, NUM_GROUP_ADDRESS_VALUE_MIN,
                            NUM_GROUP_ADDRESS_VALUE_MAX,
                            sizeof p->twdInitParams.tMacAddrFilter.numOfMacAddresses, 
                            (TI_UINT8*) &p->twdInitParams.tMacAddrFilter.numOfMacAddresses);
    
    /*printk("\nOsRgstry Num Of Group Addr:%d \n" , p->twdInitParams.tMacAddrFilter.numOfMacAddresses) ;*/

    macIndex = p->twdInitParams.tMacAddrFilter.numOfMacAddresses -1 ;
    switch( macIndex )
    {
    case 7:
        {

        regReadStringParameter(pAdapter, &STRGroup_addr7,
                            (TI_INT8*)(defStaMacAddress7), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress,(TI_UINT8*) &p->twdInitParams.tMacAddrFilter.macAddrTable[7]);       
        --macIndex;
        }

    case 6:
        {

        regReadStringParameter(pAdapter, &STRGroup_addr6,
                            (TI_INT8*)(defStaMacAddress6), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress,(TI_UINT8*) &p->twdInitParams.tMacAddrFilter.macAddrTable[6]);   
        --macIndex;
        }

    case 5: 
        {

        regReadStringParameter(pAdapter, &STRGroup_addr5,
                            (TI_INT8*)(defStaMacAddress5), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress,(TI_UINT8*) &p->twdInitParams.tMacAddrFilter.macAddrTable[5]);   
        --macIndex;
        }

    case 4: 
        {

        regReadStringParameter(pAdapter, &STRGroup_addr4,
                            (TI_INT8*)(defStaMacAddress4), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress,(TI_UINT8*) &p->twdInitParams.tMacAddrFilter.macAddrTable[4]);   
        --macIndex;
        }

    case 3:
        {

        regReadStringParameter(pAdapter, &STRGroup_addr3,
                            (TI_INT8*)(defStaMacAddress3), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress, (TI_UINT8*)&p->twdInitParams.tMacAddrFilter.macAddrTable[3]);       
        --macIndex;
        }

    case 2:
        {

        regReadStringParameter(pAdapter, &STRGroup_addr2,
                            (TI_INT8*)(defStaMacAddress2), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress,(TI_UINT8*) &p->twdInitParams.tMacAddrFilter.macAddrTable[2]);   
        --macIndex;
        }

    case 1:
        {

        regReadStringParameter(pAdapter, &STRGroup_addr1,
                            (TI_INT8*)(defStaMacAddress1), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress,(TI_UINT8*) &p->twdInitParams.tMacAddrFilter.macAddrTable[1]);   
        --macIndex;
        }

    
    case 0:
        {

        regReadStringParameter(pAdapter, &STRGroup_addr0,
                            (TI_INT8*)(defStaMacAddress0), REG_MAC_ADDR_STR_LEN,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);

        regConvertStringtoMACAddress(staMACAddress,(TI_UINT8*) &p->twdInitParams.tMacAddrFilter.macAddrTable[0]);   

        
        }

    default:
        {

        }
    }

 
    /************************/
    /* Read severity table */
    /**********************/

    regReadStringParameter(pAdapter, &STR_ReportSeverityTable,
                    (TI_INT8*)reportSeverityTableDefaults,
                    (TI_UINT8)REPORT_SEVERITY_MAX,
                    (TI_UINT8*)p->tReport.aSeverityTable,
                    (TI_UINT8*)&reportSeverityTableLen);


    /***********************/
    /* Read modules table */
    /*********************/

    regReadStringParameter(pAdapter, &STR_ReportModuleTable,
                    (TI_INT8*)reportModuleTableDefaults,
                    (TI_UINT8)REPORT_FILES_NUM,
                    (TI_UINT8*)p->tReport.aFileEnable,
                    (TI_UINT8*)&reportModuleTableLen);


    /*
        Default SSID should be non-Valid SSID, hence the STA will not try to connect
    */
    for(loopIndex = 0; loopIndex < MAX_SSID_LEN; loopIndex++)
            dummySsidString[loopIndex] = (loopIndex+1);


    /* rate Policy Params */
    regReadIntegerParameter(pAdapter, &STRRatePolicyUserShortRetryLimit,
                            CTRL_DATA_RATE_POLICY_USER_SHORT_RETRY_LIMIT_DEF, 
                            CTRL_DATA_RATE_POLICY_USER_SHORT_RETRY_LIMIT_MIN,
                            CTRL_DATA_RATE_POLICY_USER_SHORT_RETRY_LIMIT_MAX,
                            sizeof p->ctrlDataInitParams.ctrlDataTxRatePolicy.shortRetryLimit, 
                            (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataTxRatePolicy.shortRetryLimit);

    regReadIntegerParameter(pAdapter, &STRRatePolicyUserLongRetryLimit,
                            CTRL_DATA_RATE_POLICY_USER_LONG_RETRY_LIMIT_DEF,
                            CTRL_DATA_RATE_POLICY_USER_LONG_RETRY_LIMIT_MIN,
                            CTRL_DATA_RATE_POLICY_USER_LONG_RETRY_LIMIT_MAX,
                            sizeof p->ctrlDataInitParams.ctrlDataTxRatePolicy.longRetryLimit, 
                            (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataTxRatePolicy.longRetryLimit);

    regReadIntegerParameterHex (pAdapter, &STRRatePolicyUserEnabledRatesMaskCck, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_CCK_DEF, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MIN,
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MAX,
							 sizeof p->ctrlDataInitParams.policyEnabledRatesMaskCck, 
                            (TI_UINT8*)&p->ctrlDataInitParams.policyEnabledRatesMaskCck);

    regReadIntegerParameterHex (pAdapter, &STRRatePolicyUserEnabledRatesMaskOfdm, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_OFDM_DEF, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MIN,
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MAX,
							 sizeof p->ctrlDataInitParams.policyEnabledRatesMaskOfdm, 
                            (TI_UINT8*)&p->ctrlDataInitParams.policyEnabledRatesMaskOfdm);

    regReadIntegerParameterHex (pAdapter, &STRRatePolicyUserEnabledRatesMaskOfdmA, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_OFDMA_DEF, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MIN,
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MAX,
							 sizeof p->ctrlDataInitParams.policyEnabledRatesMaskOfdmA, 
                            (TI_UINT8*)&p->ctrlDataInitParams.policyEnabledRatesMaskOfdmA);

    regReadIntegerParameterHex (pAdapter, &STRRatePolicyUserEnabledRatesMaskOfdmN, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_OFDMN_DEF, 
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MIN,
							 CTRL_DATA_RATE_POLICY_USER_EN_DIS_MASK_MAX,
							 sizeof p->ctrlDataInitParams.policyEnabledRatesMaskOfdmN, 
                            (TI_UINT8*)&p->ctrlDataInitParams.policyEnabledRatesMaskOfdmN);


    regReadIntegerParameter(pAdapter, &STRRxEnergyDetection,
            TI_FALSE, TI_FALSE, TI_TRUE, 
            sizeof p->twdInitParams.tGeneral.halCtrlRxEnergyDetection,
            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlRxEnergyDetection);

   regReadIntegerParameter(pAdapter, &STRTxEnergyDetection,
            TI_FALSE, TI_FALSE, TI_TRUE, 
            sizeof p->twdInitParams.tGeneral.halCtrlTxEnergyDetection,
            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlTxEnergyDetection);

    regReadIntegerParameter(pAdapter, &STRTddCalibrationInterval,
            300, 1, 0xFFFFFFFF, 
            sizeof p->twdInitParams.tGeneral.tddRadioCalTimout,
            (TI_UINT8*)&p->twdInitParams.tGeneral.tddRadioCalTimout);

    regReadIntegerParameter(pAdapter, &STRCrtCalibrationInterval,
            2, 1, 0xFFFFFFFF, 
            sizeof p->twdInitParams.tGeneral.CrtRadioCalTimout,
            (TI_UINT8*)&p->twdInitParams.tGeneral.CrtRadioCalTimout);

    regReadIntegerParameter(pAdapter, &STRMacClockRate,
            80, 0, 255, 
            sizeof p->twdInitParams.tGeneral.halCtrlMacClock,
            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlMacClock);

    regReadIntegerParameter(pAdapter, &STRArmClockRate,
            80, 0, 255, 
            sizeof p->twdInitParams.tGeneral.halCtrlArmClock,
            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlArmClock);
    
    regReadIntegerParameter(pAdapter, &STRg80211DraftNumber,
            DRAFT_6_AND_LATER, DRAFT_5_AND_EARLIER, DRAFT_6_AND_LATER,
            sizeof p->siteMgrInitParams.siteMgrUseDraftNum,
            (TI_UINT8*)&p->siteMgrInitParams.siteMgrUseDraftNum);
     
    regReadIntegerParameter(pAdapter, &STRTraceBufferSize,
            /*1024, 0, 1024, sizeof(TI_UINT32), */
            16, 16, 16, 
            sizeof p->twdInitParams.tGeneral.TraceBufferSize,
            (TI_UINT8*)&p->twdInitParams.tGeneral.TraceBufferSize);
   
    regReadIntegerParameter(pAdapter, &STRPrintTrace, 
            TI_FALSE, TI_FALSE, TI_TRUE, 
            sizeof p->twdInitParams.tGeneral.bDoPrint, 
            (TI_UINT8*)&p->twdInitParams.tGeneral.bDoPrint);
   
    regReadIntegerParameter(pAdapter, &STRFirmwareDebug, 
            TI_FALSE, TI_FALSE, TI_TRUE, 
            sizeof p->twdInitParams.tGeneral.halCtrlFirmwareDebug, 
            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlFirmwareDebug);


#ifndef TIWLN_WINCE30
    regReadIntegerParameter(pAdapter, &STRHwACXAccessMethod,
                            TWD_HW_ACCESS_METHOD_DEF, TWD_HW_ACCESS_METHOD_MIN,
                            TWD_HW_ACCESS_METHOD_MAX,
                            sizeof p->twdInitParams.tGeneral.hwAccessMethod, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.hwAccessMethod);
#else
        /* Slave indirect*/
    p->twdInitParams.tGeneral.hwAccessMethod = 0;
#endif
    regReadIntegerParameter(pAdapter, &STRMaxSitesFragCollect,
                            TWD_SITE_FRAG_COLLECT_DEF, TWD_SITE_FRAG_COLLECT_MIN,
                            TWD_SITE_FRAG_COLLECT_MAX,
                            sizeof p->twdInitParams.tGeneral.maxSitesFragCollect, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.maxSitesFragCollect);

    regReadIntegerParameter(pAdapter, &STRBetEnable,
                            POWER_MGMNT_BET_ENABLE_DEF, POWER_MGMNT_BET_ENABLE_MIN,
                            POWER_MGMNT_BET_ENABLE_MAX,
                            sizeof p->PowerMgrInitParams.BetEnable, 
                            (TI_UINT8*)&p->PowerMgrInitParams.BetEnable);

    regReadIntegerParameter(pAdapter, &STRBetMaxConsecutive,
                            POWER_MGMNT_BET_MAX_CONSC_DEF, POWER_MGMNT_BET_MAX_CONSC_MIN,
                            POWER_MGMNT_BET_MAX_CONSC_MAX,
                            sizeof p->PowerMgrInitParams.MaximumConsecutiveET, 
                            (TI_UINT8*)&p->PowerMgrInitParams.MaximumConsecutiveET);

    /*--------------- Maximal time between full beacon reception ------------------*/
    regReadIntegerParameter(pAdapter, &STRMaxFullBeaconInterval,
                            POWER_MGMNT_MAX_FULL_BEACON_DEF, POWER_MGMNT_MAX_FULL_BEACON_MIN,
                            POWER_MGMNT_MAX_FULL_BEACON_MAX,
                            sizeof p->PowerMgrInitParams.MaximalFullBeaconReceptionInterval,
                            (TI_UINT8*)&p->PowerMgrInitParams.MaximalFullBeaconReceptionInterval);

    regReadIntegerParameter(pAdapter, &STRBetEnableThreshold,
                            POWER_MGMNT_BET_DISABLE_THRESHOLD_DEF, POWER_MGMNT_BET_DISABLE_THRESHOLD_MIN,
                            POWER_MGMNT_BET_DISABLE_THRESHOLD_MAX,
                            sizeof p->PowerMgrInitParams.BetEnableThreshold,
                            (TI_UINT8*)&p->PowerMgrInitParams.BetEnableThreshold);

    regReadIntegerParameter(pAdapter, &STRBetDisableThreshold,
                            HAL_CTRL_BET_DISABLE_THRESHOLD_DEF, HAL_CTRL_BET_DISABLE_THRESHOLD_MIN,
                            HAL_CTRL_BET_DISABLE_THRESHOLD_MAX,
                            sizeof p->PowerMgrInitParams.BetDisableThreshold,
                            (TI_UINT8*)&p->PowerMgrInitParams.BetDisableThreshold);

    regReadIntegerParameter(pAdapter, &STRTxFlashEnable,
                            TWD_TX_FLASH_ENABLE_DEF, TWD_TX_FLASH_ENABLE_MIN,
                            TWD_TX_FLASH_ENABLE_MAX,
                            sizeof p->twdInitParams.tGeneral.TxFlashEnable, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.TxFlashEnable);


    p->twdInitParams.tGeneral.beaconTemplateSize = sizeof(probeRspTemplate_t);
    p->twdInitParams.tGeneral.probeRequestTemplateSize = sizeof(probeReqTemplate_t);
    p->twdInitParams.tGeneral.probeResponseTemplateSize = sizeof(probeRspTemplate_t);
    p->twdInitParams.tGeneral.nullTemplateSize = sizeof(nullDataTemplate_t);
    p->twdInitParams.tGeneral.disconnTemplateSize = sizeof(disconnTemplate_t);
    p->twdInitParams.tGeneral.PsPollTemplateSize = sizeof(psPollTemplate_t);
    p->twdInitParams.tGeneral.qosNullDataTemplateSize = sizeof(QosNullDataTemplate_t);

    regReadIntegerParameter(pAdapter,
                            &STRBeaconRxTimeout,
                            BCN_RX_TIMEOUT_DEF_VALUE,
                            BCN_RX_TIMEOUT_MIN_VALUE,
                            BCN_RX_TIMEOUT_MAX_VALUE,
                            sizeof p->twdInitParams.tGeneral.BeaconRxTimeout,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.BeaconRxTimeout);

    regReadIntegerParameter(pAdapter,
                            &STRBroadcastRxTimeout,
                            BROADCAST_RX_TIMEOUT_DEF_VALUE,
                            BROADCAST_RX_TIMEOUT_MIN_VALUE,
                            BROADCAST_RX_TIMEOUT_MAX_VALUE,
                            sizeof p->twdInitParams.tGeneral.BroadcastRxTimeout,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.BroadcastRxTimeout);

    regReadIntegerParameter(pAdapter,
                            &STRRxBroadcastInPs,
                            RX_BROADCAST_IN_PS_DEF_VALUE,
                            RX_BROADCAST_IN_PS_MIN_VALUE,
                            RX_BROADCAST_IN_PS_MAX_VALUE,
                            sizeof p->twdInitParams.tGeneral.RxBroadcastInPs,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.RxBroadcastInPs);

    regReadIntegerParameter(pAdapter, &STRCalibrationChannel2_4,
                            TWD_CALIBRATION_CHANNEL_2_4_DEF, TWD_CALIBRATION_CHANNEL_2_4_MIN,
                            TWD_CALIBRATION_CHANNEL_2_4_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlCalibrationChannel2_4, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlCalibrationChannel2_4);

    regReadIntegerParameter(pAdapter, &STRCalibrationChannel5_0,
                            TWD_CALIBRATION_CHANNEL_5_0_DEF, TWD_CALIBRATION_CHANNEL_5_0_MIN,
                            TWD_CALIBRATION_CHANNEL_5_0_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlCalibrationChannel5_0, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlCalibrationChannel5_0);

    regReadIntegerParameter(pAdapter,
                            &STRConsecutivePsPollDeliveryFailureThreshold,
                            CONSECUTIVE_PS_POLL_FAILURE_DEF,
                            CONSECUTIVE_PS_POLL_FAILURE_MIN,
                            CONSECUTIVE_PS_POLL_FAILURE_MAX,
                            sizeof p->twdInitParams.tGeneral.ConsecutivePsPollDeliveryFailureThreshold,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.ConsecutivePsPollDeliveryFailureThreshold);


    regReadIntegerParameter(pAdapter, &STRdot11RTSThreshold,
                            TWD_RTS_THRESHOLD_DEF, TWD_RTS_THRESHOLD_MIN,
                            TWD_RTS_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlRtsThreshold, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlRtsThreshold);

    regReadIntegerParameter(pAdapter, &STRRxDisableBroadcast,
                            TWD_RX_DISABLE_BROADCAST_DEF, TWD_RX_DISABLE_BROADCAST_MIN,
                            TWD_RX_DISABLE_BROADCAST_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlRxDisableBroadcast, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlRxDisableBroadcast);

    regReadIntegerParameter(pAdapter, &STRRecoveryEnable,
                            TWD_RECOVERY_ENABLE_DEF, TWD_RECOVERY_ENABLE_MIN,
                            TWD_RECOVERY_ENABLE_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlRecoveryEnable, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlRecoveryEnable);

    p->healthMonitorInitParams.FullRecoveryEnable = (TI_BOOL)p->twdInitParams.tGeneral.halCtrlRecoveryEnable;

    regReadIntegerParameter(pAdapter, &STRdot11FragThreshold,
                            TWD_FRAG_THRESHOLD_DEF, TWD_FRAG_THRESHOLD_MIN,
                            TWD_FRAG_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlFragThreshold, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlFragThreshold);

    regReadIntegerParameter(pAdapter, &STRdot11MaxTxMSDULifetime,
                            TWD_MAX_TX_MSDU_LIFETIME_DEF, TWD_MAX_TX_MSDU_LIFETIME_MIN,
                            TWD_MAX_TX_MSDU_LIFETIME_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlMaxTxMsduLifetime, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlMaxTxMsduLifetime);

    regReadIntegerParameter(pAdapter, &STRdot11MaxReceiveLifetime,
                            TWD_MAX_RX_MSDU_LIFETIME_DEF, TWD_MAX_RX_MSDU_LIFETIME_MIN,
                            TWD_MAX_RX_MSDU_LIFETIME_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlMaxRxMsduLifetime, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlMaxRxMsduLifetime);
    
    regReadIntegerParameter(pAdapter, &STRdot11RateFallBackRetryLimit,
                            TWD_RATE_FB_RETRY_LIMIT_DEF, TWD_RATE_FB_RETRY_LIMIT_MIN,
                            TWD_RATE_FB_RETRY_LIMIT_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlRateFallbackRetry, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlRateFallbackRetry);

    regReadIntegerParameter(pAdapter, &STRListenInterval,
                            TWD_LISTEN_INTERVAL_DEF, TWD_LISTEN_INTERVAL_MIN,
                            TWD_LISTEN_INTERVAL_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlListenInterval, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlListenInterval);

    regReadIntegerParameter(pAdapter, &STRdot11TxAntenna,
                            TWD_TX_ANTENNA_DEF, TWD_TX_ANTENNA_MIN,
                            TWD_TX_ANTENNA_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlTxAntenna, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlTxAntenna);
    /* reverse tx antenna value - ACX and utility have reversed values */
    if (p->twdInitParams.tGeneral.halCtrlTxAntenna == TX_ANTENNA_2)
       p->twdInitParams.tGeneral.halCtrlTxAntenna = TX_ANTENNA_1;
    else
       p->twdInitParams.tGeneral.halCtrlTxAntenna = TX_ANTENNA_2;


    regReadIntegerParameter(pAdapter, &STRdot11RxAntenna,
                            TWD_RX_ANTENNA_DEF, TWD_RX_ANTENNA_MIN,
                            TWD_RX_ANTENNA_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlRxAntenna, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlRxAntenna);


    regReadIntegerParameter(pAdapter, &STRTxCompleteThreshold,
                            TWD_TX_CMPLT_THRESHOLD_DEF, TWD_TX_CMPLT_THRESHOLD_MIN,
                            TWD_TX_CMPLT_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.TxCompletePacingThreshold, 
                            (TI_UINT8*)&(p->twdInitParams.tGeneral.TxCompletePacingThreshold));

    regReadIntegerParameter(pAdapter, &STRTxCompleteTimeout,
                            TWD_TX_CMPLT_TIMEOUT_DEF, TWD_TX_CMPLT_TIMEOUT_MIN,
                            TWD_TX_CMPLT_TIMEOUT_MAX,
                            sizeof p->twdInitParams.tGeneral.TxCompletePacingTimeout, 
                            (TI_UINT8*)&(p->twdInitParams.tGeneral.TxCompletePacingTimeout));

    regReadIntegerParameter(pAdapter, &STRRxInterruptThreshold,
                            TWD_RX_INTR_THRESHOLD_DEF, TWD_RX_INTR_THRESHOLD_MIN,
                            TWD_RX_INTR_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.RxIntrPacingThreshold, 
                            (TI_UINT8*)&(p->twdInitParams.tGeneral.RxIntrPacingThreshold));

    regReadIntegerParameter(pAdapter, &STRRxInterruptTimeout,
                            TWD_RX_INTR_TIMEOUT_DEF, TWD_RX_INTR_TIMEOUT_MIN,
                            TWD_RX_INTR_TIMEOUT_MAX,
                            sizeof p->twdInitParams.tGeneral.RxIntrPacingTimeout, 
                            (TI_UINT8*)&(p->twdInitParams.tGeneral.RxIntrPacingTimeout));


    regReadIntegerParameter(pAdapter, &STRdot11DesiredChannel,
                        SITE_MGR_CHANNEL_DEF, SITE_MGR_CHANNEL_MIN, SITE_MGR_CHANNEL_MAX,
                        sizeof p->siteMgrInitParams.siteMgrDesiredChannel, 
                        (TI_UINT8*)&p->siteMgrInitParams.siteMgrDesiredChannel);

    /* NOTE: desired BSSID and SSID (and BSS type, later on) are currently set both to the SME and the site manager!!! */
    MAC_COPY (p->siteMgrInitParams.siteMgrDesiredBSSID, bssidBroadcast);
    MAC_COPY (p->tSmeModifiedInitParams.tDesiredBssid, bssidBroadcast);

    regReadStringParameter(pAdapter, &STRdot11DesiredSSID,
                    (TI_INT8*)dummySsidString,
                    (TI_UINT8)MAX_SSID_LEN,
                    (TI_UINT8*)p->siteMgrInitParams.siteMgrDesiredSSID.str,
                    (TI_UINT8*)&p->siteMgrInitParams.siteMgrDesiredSSID.len);

    /* in case no SSID desired at the init file set dummy len */
    for(loopIndex = 0; loopIndex < MAX_SSID_LEN; loopIndex++)
    {
        if(dummySsidString[loopIndex] != p->siteMgrInitParams.siteMgrDesiredSSID.str[loopIndex])
            break;
    }

    p->siteMgrInitParams.siteMgrDesiredSSID.len = (TI_UINT8)loopIndex;

    NdisMoveMemory(&p->tSmeModifiedInitParams.tDesiredSsid,
                   &p->siteMgrInitParams.siteMgrDesiredSSID, 
                   sizeof(TSsid));

    regReadIntegerParameter(pAdapter, &STRdot11DesiredNetworkType,
                            SITE_MGR_DOT_11_MODE_DEF, SITE_MGR_DOT_11_MODE_MIN, SITE_MGR_DOT_11_MODE_MAX,
                            sizeof p->siteMgrInitParams.siteMgrDesiredDot11Mode, 
                            (TI_UINT8*)&p->siteMgrInitParams.siteMgrDesiredDot11Mode);

    regReadIntegerParameter(pAdapter, &STRdot11SlotTime,
        PHY_SLOT_TIME_SHORT, PHY_SLOT_TIME_LONG, PHY_SLOT_TIME_SHORT,
        sizeof p->siteMgrInitParams.siteMgrDesiredSlotTime,
        (TI_UINT8*)&p->siteMgrInitParams.siteMgrDesiredSlotTime);

    regReadIntegerParameter(pAdapter, &STRdot11RtsCtsProtection,
        0, 0, 1, 
        sizeof p->ctrlDataInitParams.ctrlDataDesiredCtsRtsStatus,
        (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataDesiredCtsRtsStatus);

    regReadIntegerParameter(pAdapter, &STRdot11IbssProtection,
        ERP_PROTECTION_STANDARD, ERP_PROTECTION_NONE, ERP_PROTECTION_STANDARD,
        sizeof p->ctrlDataInitParams.ctrlDataDesiredIbssProtection,
        (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataDesiredIbssProtection);

    /* When working in band A, minimum channel is 36 and not 1*/
    if (p->siteMgrInitParams.siteMgrDesiredDot11Mode  == DOT11_A_MODE)
    {
        if (p->siteMgrInitParams.siteMgrDesiredChannel < SITE_MGR_CHANNEL_A_MIN)
            p->siteMgrInitParams.siteMgrDesiredChannel = SITE_MGR_CHANNEL_A_MIN;
    }

    if(1)
    {
        TI_UINT32 Freq2ChannelTable[] = {0,2412000,2417000,2422000,2427000,2432000,2437000,
                                      2442000,2447000,2452000,2457000,
                                      2462000,2467000,2472000,2484000};

        memcpy(p->siteMgrInitParams.siteMgrFreq2ChannelTable,
               Freq2ChannelTable,
               SITE_MGR_CHANNEL_MAX+1);
    }

    /* read TX rates from registry */
    readRates(pAdapter, p);

    /* Note: desired BSS type is set both to the SME and site manager */
    regReadIntegerParameter(pAdapter, &STRdot11DesiredBSSType,
                            SITE_MGR_BSS_TYPE_DEF, BSS_INDEPENDENT, BSS_ANY,
                            sizeof p->siteMgrInitParams.siteMgrDesiredBSSType,
                            (TI_UINT8*)&p->siteMgrInitParams.siteMgrDesiredBSSType);
    p->tSmeModifiedInitParams.eDesiredBssType = p->siteMgrInitParams.siteMgrDesiredBSSType;

    regReadIntegerParameter(pAdapter, &STRdot11BeaconPeriod,
                            SITE_MGR_BEACON_INTERVAL_DEF, SITE_MGR_BEACON_INTERVAL_MIN,
                            SITE_MGR_BEACON_INTERVAL_MAX,
                            sizeof p->siteMgrInitParams.siteMgrDesiredBeaconInterval,
                            (TI_UINT8*)&p->siteMgrInitParams.siteMgrDesiredBeaconInterval);

    regReadIntegerParameter(pAdapter, &STRdot11ShortPreambleInvoked,
                            SITE_MGR_PREAMBLE_TYPE_DEF, PREAMBLE_LONG, PREAMBLE_SHORT,
                            sizeof p->siteMgrInitParams.siteMgrDesiredPreambleType,
                            (TI_UINT8*)&p->siteMgrInitParams.siteMgrDesiredPreambleType);

    regReadIntegerParameter(pAdapter, &STRExternalMode,
                            SITE_MGR_EXTERNAL_MODE_DEF, SITE_MGR_EXTERNAL_MODE_MIN,
                            SITE_MGR_EXTERNAL_MODE_MAX,
                            sizeof p->siteMgrInitParams.siteMgrExternalConfiguration,
                            (TI_UINT8*)&p->siteMgrInitParams.siteMgrExternalConfiguration);

    regReadIntegerParameter(pAdapter, &STRWiFiAdHoc,
                            SITE_MGR_WiFiAdHoc_DEF, SITE_MGR_WiFiAdHoc_MIN, SITE_MGR_WiFiAdHoc_MAX,
                            sizeof p->siteMgrInitParams.siteMgrWiFiAdhoc,
                            (TI_UINT8*)&p->siteMgrInitParams.siteMgrWiFiAdhoc);

    regReadIntegerParameter(pAdapter, &STRWiFiWmmPS,
                            WIFI_WMM_PS_DEF, WIFI_WMM_PS_MIN, WIFI_WMM_PS_MAX,
                            sizeof p->twdInitParams.tGeneral.WiFiWmmPS,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.WiFiWmmPS);

    regReadIntegerParameter(pAdapter, &STRConnSelfTimeout,
                            CONN_SELF_TIMEOUT_DEF, CONN_SELF_TIMEOUT_MIN, CONN_SELF_TIMEOUT_MAX,
                            sizeof p->connInitParams.connSelfTimeout,
                            (TI_UINT8*)&p->connInitParams.connSelfTimeout);

    regReadIntegerParameter(pAdapter, &STRdot11AuthRespTimeout,
                            AUTH_RESPONSE_TIMEOUT_DEF, AUTH_RESPONSE_TIMEOUT_MIN, AUTH_RESPONSE_TIMEOUT_MAX,
                            sizeof p->authInitParams.authResponseTimeout,
                            (TI_UINT8*)&p->authInitParams.authResponseTimeout);

    regReadIntegerParameter(pAdapter, &STRdot11MaxAuthRetry,
                            AUTH_MAX_RETRY_COUNT_DEF, AUTH_MAX_RETRY_COUNT_MIN, AUTH_MAX_RETRY_COUNT_MAX,
                            sizeof p->authInitParams.authMaxRetryCount,
                            (TI_UINT8*)&p->authInitParams.authMaxRetryCount);

    regReadIntegerParameter(pAdapter, &STRdot11AssocRespTimeout,
                            ASSOC_RESPONSE_TIMEOUT_DEF, ASSOC_RESPONSE_TIMEOUT_MIN, ASSOC_RESPONSE_TIMEOUT_MAX,
                            sizeof p->assocInitParams.assocResponseTimeout,
                            (TI_UINT8*)&p->assocInitParams.assocResponseTimeout);

    regReadIntegerParameter(pAdapter, &STRdot11MaxAssocRetry,
                            ASSOC_MAX_RETRY_COUNT_DEF, ASSOC_MAX_RETRY_COUNT_MIN, ASSOC_MAX_RETRY_COUNT_MAX,
                            sizeof p->assocInitParams.assocMaxRetryCount,
                            (TI_UINT8*)&p->assocInitParams.assocMaxRetryCount);

    regReadIntegerParameter(pAdapter, &STRRxDataFiltersEnabled,
                            RX_DATA_FILTERS_ENABLED_DEF, RX_DATA_FILTERS_ENABLED_MIN, RX_DATA_FILTERS_ENABLED_MAX,
                            sizeof p->rxDataInitParams.rxDataFiltersEnabled,
                            (TI_UINT8*)&p->rxDataInitParams.rxDataFiltersEnabled);

    regReadIntegerParameter(pAdapter, &STRRxDataFiltersFilter1Offset,
                            RX_DATA_FILTERS_FILTER_OFFSET_DEF, RX_DATA_FILTERS_FILTER_OFFSET_MIN, RX_DATA_FILTERS_FILTER_OFFSET_MAX,
                            sizeof filterOffset,
                            (TI_UINT8*) &filterOffset);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter1Mask,
                            RX_DATA_FILTERS_FILTER_MASK_DEF, RX_DATA_FILTERS_FILTER_MASK_LEN_DEF,
                            (TI_UINT8*) filterMask,
                            (TI_UINT8*) &filterMaskLength);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter1Pattern,
                            RX_DATA_FILTERS_FILTER_PATTERN_DEF, RX_DATA_FILTERS_FILTER_PATTERN_LEN_DEF,
                            (TI_UINT8*) filterPattern,
                            (TI_UINT8*) &filterPatternLength);

    parse_filter_request(&p->rxDataInitParams.rxDataFilterRequests[0], filterOffset, filterMask, filterMaskLength, filterPattern, filterPatternLength);

    regReadIntegerParameter(pAdapter, &STRRxDataFiltersFilter2Offset,
                            RX_DATA_FILTERS_FILTER_OFFSET_DEF, RX_DATA_FILTERS_FILTER_OFFSET_MIN, RX_DATA_FILTERS_FILTER_OFFSET_MAX,
                            sizeof filterOffset,
                            (TI_UINT8*) &filterOffset);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter2Mask,
                            RX_DATA_FILTERS_FILTER_MASK_DEF, RX_DATA_FILTERS_FILTER_MASK_LEN_DEF,
                            (TI_UINT8*) filterMask,
                            (TI_UINT8*) &filterMaskLength);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter2Pattern,
                            RX_DATA_FILTERS_FILTER_PATTERN_DEF, RX_DATA_FILTERS_FILTER_PATTERN_LEN_DEF,
                            (TI_UINT8*) filterPattern,
                            (TI_UINT8*) &filterPatternLength);

    parse_filter_request(&p->rxDataInitParams.rxDataFilterRequests[1], filterOffset, filterMask, filterMaskLength, filterPattern, filterPatternLength);

    regReadIntegerParameter(pAdapter, &STRRxDataFiltersFilter3Offset,
                            RX_DATA_FILTERS_FILTER_OFFSET_DEF, RX_DATA_FILTERS_FILTER_OFFSET_MIN, RX_DATA_FILTERS_FILTER_OFFSET_MAX,
                            sizeof filterOffset,
                            (TI_UINT8*) &filterOffset);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter3Mask,
                            RX_DATA_FILTERS_FILTER_MASK_DEF, RX_DATA_FILTERS_FILTER_MASK_LEN_DEF,
                            (TI_UINT8*) filterMask,
                            (TI_UINT8*) &filterMaskLength);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter3Pattern,
                            RX_DATA_FILTERS_FILTER_PATTERN_DEF, RX_DATA_FILTERS_FILTER_PATTERN_LEN_DEF,
                            (TI_UINT8*) filterPattern,
                            (TI_UINT8*) &filterPatternLength);

    parse_filter_request(&p->rxDataInitParams.rxDataFilterRequests[2], filterOffset, filterMask, filterMaskLength, filterPattern, filterPatternLength);

    regReadIntegerParameter(pAdapter, &STRRxDataFiltersFilter4Offset,
                            RX_DATA_FILTERS_FILTER_OFFSET_DEF, RX_DATA_FILTERS_FILTER_OFFSET_MIN, RX_DATA_FILTERS_FILTER_OFFSET_MAX,
                            sizeof filterOffset,
                            (TI_UINT8*) &filterOffset);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter4Mask,
                            RX_DATA_FILTERS_FILTER_MASK_DEF, RX_DATA_FILTERS_FILTER_MASK_LEN_DEF,
                            (TI_UINT8*) filterMask,
                            (TI_UINT8*) &filterMaskLength);

    regReadStringParameter(pAdapter, &STRRxDataFiltersFilter4Pattern,
                            RX_DATA_FILTERS_FILTER_PATTERN_DEF, RX_DATA_FILTERS_FILTER_PATTERN_LEN_DEF,
                            (TI_UINT8*) filterPattern,
                            (TI_UINT8*) &filterPatternLength);

    parse_filter_request(&p->rxDataInitParams.rxDataFilterRequests[3], filterOffset, filterMask, filterMaskLength, filterPattern, filterPatternLength);

    regReadIntegerParameter(pAdapter, &STRRxDataFiltersDefaultAction,
                            RX_DATA_FILTERS_DEFAULT_ACTION_DEF, RX_DATA_FILTERS_DEFAULT_ACTION_MIN,
                            RX_DATA_FILTERS_DEFAULT_ACTION_MAX,
                            sizeof p->rxDataInitParams.rxDataFiltersDefaultAction,
                            (TI_UINT8*)&p->rxDataInitParams.rxDataFiltersDefaultAction);

    regReadIntegerParameter(pAdapter, &STRCreditCalcTimout,
                            TX_DATA_CREDIT_CALC_TIMOEUT_DEF, TX_DATA_CREDIT_CALC_TIMOEUT_MIN,
                            TX_DATA_CREDIT_CALC_TIMOEUT_MAX,
                            sizeof p->txDataInitParams.creditCalculationTimeout,
                            (TI_UINT8*)&p->txDataInitParams.creditCalculationTimeout);

	regReadIntegerParameter(pAdapter, &STRCreditCalcTimerEnabled,
                            TI_FALSE, TI_FALSE, TI_TRUE,
							sizeof p->txDataInitParams.bCreditCalcTimerEnabled,
							(TI_UINT8*)&p->txDataInitParams.bCreditCalcTimerEnabled);

    regReadIntegerParameter(pAdapter, &STRTrafficAdmControlTimeout,
                    TRAFFIC_ADM_CONTROL_TIMEOUT_DEF, TRAFFIC_ADM_CONTROL_TIMEOUT_MIN,
                    TRAFFIC_ADM_CONTROL_TIMEOUT_MAX,
                    sizeof p->qosMngrInitParams.trafficAdmCtrlInitParams.trafficAdmCtrlResponseTimeout,
                    (TI_UINT8*)&p->qosMngrInitParams.trafficAdmCtrlInitParams.trafficAdmCtrlResponseTimeout);

    regReadIntegerParameter(pAdapter, &STRTrafficAdmControlUseFixedMsduSize,
                    TI_FALSE, TI_FALSE, TI_TRUE,
                    sizeof p->qosMngrInitParams.trafficAdmCtrlInitParams.trafficAdmCtrlUseFixedMsduSize,
                    (TI_UINT8*)&p->qosMngrInitParams.trafficAdmCtrlInitParams.trafficAdmCtrlUseFixedMsduSize);

    regReadIntegerParameter(pAdapter, &STRDesiredMaxSpLen,
                    QOS_MAX_SP_LEN_DEF, QOS_MAX_SP_LEN_MIN,
                    QOS_MAX_SP_LEN_MAX,
                    sizeof p->qosMngrInitParams.desiredMaxSpLen,
                    (TI_UINT8*)&p->qosMngrInitParams.desiredMaxSpLen);

    regReadIntegerParameter(pAdapter, &STRCwFromUserEnable,
                    QOS_CW_USER_ENABLE_DEF, QOS_CW_USER_ENABLE_MIN,
                    QOS_CW_USER_ENABLE_MAX,
                    sizeof p->qosMngrInitParams.bCwFromUserEnable,
                    (TI_UINT8*)&p->qosMngrInitParams.bCwFromUserEnable);

    regReadIntegerParameter(pAdapter, &STRDesireCwMin,
                    QOS_CW_CWMIN_DEF, QOS_CW_CWMIN_MIN,
                    QOS_CW_CWMIN_MAX,
                    sizeof p->qosMngrInitParams.uDesireCwMin,
                    (TI_UINT8*)&p->qosMngrInitParams.uDesireCwMin);
    
    regReadIntegerParameter(pAdapter, &STRDesireCwMax,
                    QOS_CW_CWMAX_DEF, QOS_CW_CWMAX_MIN,
                    QOS_CW_CWMAX_MAX,
                    sizeof p->qosMngrInitParams.uDesireCwMax,
                    (TI_UINT8*)&p->qosMngrInitParams.uDesireCwMax);

/*                              SME Initialization Parameters                           */
/*                          ====================================                        */

    regReadIntegerParameter(pAdapter, &STRSmeRadioOn,
                            TI_TRUE, TI_FALSE, TI_TRUE,
                            sizeof p->tSmeModifiedInitParams.bRadioOn,
                            (TI_UINT8*)&p->tSmeModifiedInitParams.bRadioOn);
    regReadIntegerParameter(pAdapter, &STRSmeConnectMode,
                            CONNECT_MODE_AUTO, CONNECT_MODE_AUTO, CONNECT_MODE_MANUAL,
                            sizeof p->tSmeModifiedInitParams.eConnectMode,
                            (TI_UINT8*)&p->tSmeModifiedInitParams.eConnectMode);

    {
        /* due to the fact that two following init keys has negative values, we read them as table */  
        TI_UINT32   uEntriesNumber = 1;

        regReadIntegerTable(pAdapter ,&STRSmeScanRssiThreshold,
                            SME_SCAN_RSSI_THRESHOLD_DEF, sizeof(TI_INT8) * 3,
                            (TI_UINT8*)&p->tSmeInitParams.iRssiThreshold, NULL, &uEntriesNumber, 
                            sizeof(TI_INT8),TI_FALSE);

        if ( (p->tSmeInitParams.iRssiThreshold < SME_SCAN_RSSI_THRESHOLD_MIN) ||
             (p->tSmeInitParams.iRssiThreshold > SME_SCAN_RSSI_THRESHOLD_MAX))
        {
            p->tSmeInitParams.iRssiThreshold = SME_SCAN_RSSI_THRESHOLD_DEF_NUM;
        }

        regReadIntegerTable(pAdapter ,&STRSmeScanSnrThreshold,
                               SME_SCAN_SNR_THRESHOLD_DEF, sizeof(TI_INT8),
                               (TI_UINT8*)&p->tSmeInitParams.iSnrThreshold, NULL, &uEntriesNumber, sizeof(TI_INT8),TI_FALSE);
 
        if ( (p->tSmeInitParams.iSnrThreshold < SME_SCAN_SNR_THRESHOLD_MIN) ||
             (p->tSmeInitParams.iSnrThreshold > SME_SCAN_SNR_THRESHOLD_MAX))
        {
            p->tSmeInitParams.iSnrThreshold = SME_SCAN_SNR_THRESHOLD_DEF_NUM;
        }
    }

    regReadIntegerParameter(pAdapter, &STRSmeScanCycleNumber,
                            SME_SCAN_CYCLES_DEF, SME_SCAN_CYCLES_MIN, SME_SCAN_CYCLES_MAX,
                            sizeof p->tSmeInitParams.uCycleNum,
                            (TI_UINT8*)&p->tSmeInitParams.uCycleNum);
    regReadIntegerParameter(pAdapter, &STRSmeScanMaxDwellTime,
                            SME_SCAN_MAX_DWELL_DEF, SME_SCAN_MAX_DWELL_MIN, SME_SCAN_MAX_DWELL_MAX,
                            sizeof p->tSmeInitParams.uMaxScanDuration,
                            (TI_UINT8*)&p->tSmeInitParams.uMaxScanDuration);
    regReadIntegerParameter(pAdapter, &STRSmeScanMinDwellTime,
                            SME_SCAN_MIN_DWELL_DEF, SME_SCAN_MIN_DWELL_MIN, SME_SCAN_MIN_DWELL_MAX,
                            sizeof p->tSmeInitParams.uMinScanDuration,
                            (TI_UINT8*)&p->tSmeInitParams.uMinScanDuration);
    regReadIntegerParameter(pAdapter, &STRSmeScanProbeRequestNumber,
                            SME_SCAN_PROBE_REQ_DEF, SME_SCAN_PROBE_REQ_MIN, SME_SCAN_PROBE_REQ_MAX,
                            sizeof p->tSmeInitParams.uProbeReqNum,
                            (TI_UINT8*)&p->tSmeInitParams.uProbeReqNum);
    regReadIntegerTable(pAdapter, &STRSmeScanIntervals, SME_SCAN_INTERVALS_LIST_VAL_DEF,
                        SME_SCAN_INTERVALS_LIST_STRING_MAX_SIZE,
                        (TI_UINT8*)&uSmeScanIntervalsTempList, NULL, &uTempEntriesCount,
                        sizeof (TI_UINT32),TI_FALSE);
    /* sanity check */
    if (uTempEntriesCount > PERIODIC_SCAN_MAX_INTERVAL_NUM)
    {
        uTempEntriesCount = PERIODIC_SCAN_MAX_INTERVAL_NUM;
    }
    /* convert from TI_UINT8 to TI_UINT32 */
    for (uIndex = 0; uIndex < uTempEntriesCount; uIndex++)
    {
        p->tSmeInitParams.uScanIntervals[ uIndex ] = uSmeScanIntervalsTempList[ uIndex ];
    }
    regReadIntegerTable(pAdapter, &STRSmeScanGChannels, SME_SCAN_CHANNELS_LIST_G_VAL_DEF,
                        SME_SCAN_CHANNELS_LIST_G_STRING_MAX_SIZE,
                        (TI_UINT8*)&uSmeTempList, NULL, &uTempEntriesCount,
                        sizeof (TI_UINT8),TI_FALSE);


    /* convert to channel list */
    for (uIndex = 0; uIndex < uTempEntriesCount; uIndex++)
    {
        p->tSmeInitParams.tChannelList[ uIndex ].eBand = RADIO_BAND_2_4_GHZ;
        p->tSmeInitParams.tChannelList[ uIndex ].uChannel = uSmeTempList[ uIndex ];
    }
    uSmeGChannelsCount = uTempEntriesCount;

	/* 
	 * Add A_MODE channels to scan list only if it enabled 
	 * NOTE: Don't use empty channel list string 
	 */
    if ((p->siteMgrInitParams.siteMgrDesiredDot11Mode  == DOT11_A_MODE) ||
        (p->siteMgrInitParams.siteMgrDesiredDot11Mode  == DOT11_DUAL_MODE))
    {
		regReadIntegerTable(pAdapter, &STRSmeScanAChannels, SME_SCAN_CHANNELS_LIST_A_VAL_DEF,
							SME_SCAN_CHANNELS_LIST_A_STRING_MAX_SIZE,
							(TI_UINT8*)&uSmeTempList, NULL, &uTempEntriesCount,
							sizeof (TI_UINT8),TI_FALSE);

		/* convert to channel list */
		for (uIndex = 0; uIndex < uTempEntriesCount; uIndex++)
		{
			p->tSmeInitParams.tChannelList[ uSmeGChannelsCount + uIndex ].eBand = RADIO_BAND_5_0_GHZ;
			p->tSmeInitParams.tChannelList[ uSmeGChannelsCount + uIndex ].uChannel = uSmeTempList[ uIndex ];
		}

		p->tSmeInitParams.uChannelNum = uSmeGChannelsCount + uIndex;
	}
	else
	{
		p->tSmeInitParams.uChannelNum = uSmeGChannelsCount;
	}

    regReadIntegerParameter(pAdapter, &STRdot11AuthenticationMode,
                            RSN_AUTH_SUITE_DEF, RSN_AUTH_SUITE_MIN, RSN_AUTH_SUITE_MAX,
                            sizeof p->rsnInitParams.authSuite,
                            (TI_UINT8*)&p->rsnInitParams.authSuite);

    /* Soft Gemini Section */
    
    regReadIntegerParameter(pAdapter, &STRBThWlanCoexistEnable,
                            SOFT_GEMINI_ENABLED_DEF, SOFT_GEMINI_ENABLED_MIN, SOFT_GEMINI_ENABLED_MAX,
                            sizeof p->SoftGeminiInitParams.SoftGeminiEnable, 
                            (TI_UINT8*)&p->SoftGeminiInitParams.SoftGeminiEnable);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistParamsBtLoadRatio,
                            SOFT_GEMINI_PARAMS_LOAD_RATIO_DEF, SOFT_GEMINI_PARAMS_LOAD_RATIO_MIN, SOFT_GEMINI_PARAMS_LOAD_RATIO_MAX,
                            sizeof p->SoftGeminiInitParams.coexBtLoadRatio,
                            (TI_UINT8*)&p->SoftGeminiInitParams.coexBtLoadRatio);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistParamsAutoPsMode,
                            SOFT_GEMINI_PARAMS_AUTO_PS_MODE_DEF, SOFT_GEMINI_PARAMS_AUTO_PS_MODE_MIN, SOFT_GEMINI_PARAMS_AUTO_PS_MODE_MAX,
                            sizeof p->SoftGeminiInitParams.coexAutoPsMode,
                            (TI_UINT8*)&p->SoftGeminiInitParams.coexAutoPsMode);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistParamsAutoScanProbeReqPerc,
                            SOFT_GEMINI_PARAMS_AUTO_SCAN_PROBE_REQ_DEF, SOFT_GEMINI_PARAMS_AUTO_SCAN_PROBE_REQ_MIN, SOFT_GEMINI_PARAMS_AUTO_SCAN_PROBE_REQ_MAX,
                            sizeof p->SoftGeminiInitParams.coexAutoScanEnlargedNumOfProbeReqPercent,
                            (TI_UINT8*)&p->SoftGeminiInitParams.coexAutoScanEnlargedNumOfProbeReqPercent);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistParamsAutoScanWindowPerc,
                            SOFT_GEMINI_PARAMS_AUTO_SCAN_WINDOW_DEF, SOFT_GEMINI_PARAMS_AUTO_SCAN_WINDOW_MIN, SOFT_GEMINI_PARAMS_AUTO_SCAN_WINDOW_MAX,
                            sizeof p->SoftGeminiInitParams.coexAutoScanEnlargedScanWindowPercent,
                            (TI_UINT8*)&p->SoftGeminiInitParams.coexAutoScanEnlargedScanWindowPercent);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistParamsAutoScanMaxTime,
							SOFT_GEMINI_AUTO_SCAN_COMPENSATION_MAX_TIME_DEF, SOFT_GEMINI_AUTO_SCAN_COMPENSATION_MAX_TIME_MIN, SOFT_GEMINI_AUTO_SCAN_COMPENSATION_MAX_TIME_MAX,
							sizeof p->SoftGeminiInitParams.coexAutoScanCompensationMaxTime,
							(TI_UINT8*)&p->SoftGeminiInitParams.coexAutoScanCompensationMaxTime);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistPerThreshold,
							SOFT_GEMINI_PARAMS_PER_THRESHOLD_DEF, SOFT_GEMINI_PARAMS_PER_THRESHOLD_MIN, SOFT_GEMINI_PARAMS_PER_THRESHOLD_MAX,
							sizeof p->SoftGeminiInitParams.coexBtPerThreshold,
							(TI_UINT8*)&p->SoftGeminiInitParams.coexBtPerThreshold);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistNfsSampleInterval,
							SOFT_GEMINI_PARAMS_NFS_SAMPLE_INTERVAL_DEF, SOFT_GEMINI_PARAMS_NFS_SAMPLE_INTERVAL_MIN, SOFT_GEMINI_PARAMS_NFS_SAMPLE_INTERVAL_MAX,
							sizeof p->SoftGeminiInitParams.coexBtNfsSampleInterval,
							(TI_UINT8*)&p->SoftGeminiInitParams.coexBtNfsSampleInterval);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistcoexAntennaConfiguration,
							SOFT_GEMINI_ANTENNA_CONFIGURATION_DEF, SOFT_GEMINI_ANTENNA_CONFIGURATION_MIN, SOFT_GEMINI_ANTENNA_CONFIGURATION_MAX,
							sizeof p->SoftGeminiInitParams.coexAntennaConfiguration,
							(TI_UINT8*)&p->SoftGeminiInitParams.coexAntennaConfiguration);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistcoexMaxConsecutiveBeaconMissPrecent,
							SOFT_GEMINI_BEACON_MISS_PERCENT_DEF, SOFT_GEMINI_BEACON_MISS_PERCENT_MIN, SOFT_GEMINI_BEACON_MISS_PERCENT_MAX,
							sizeof p->SoftGeminiInitParams.coexMaxConsecutiveBeaconMissPrecent,
							(TI_UINT8*)&p->SoftGeminiInitParams.coexMaxConsecutiveBeaconMissPrecent);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistcoexAPRateAdapationThr,
							SOFT_GEMINI_RATE_ADAPT_THRESH_DEF, SOFT_GEMINI_RATE_ADAPT_THRESH_MIN, SOFT_GEMINI_RATE_ADAPT_THRESH_MAX,
							sizeof p->SoftGeminiInitParams.coexAPRateAdapationThr,
							(TI_UINT8*)&p->SoftGeminiInitParams.coexAPRateAdapationThr);

	regReadIntegerParameter(pAdapter, &STRBThWlanCoexistcoexAPRateAdapationSnr,
							SOFT_GEMINI_RATE_ADAPT_SNR_DEF, SOFT_GEMINI_RATE_ADAPT_SNR_MIN, SOFT_GEMINI_RATE_ADAPT_SNR_MAX,
							sizeof p->SoftGeminiInitParams.coexAPRateAdapationSnr,
							(TI_UINT8*)&p->SoftGeminiInitParams.coexAPRateAdapationSnr);
    /* 
     * CoexActivity table 
     */

    /* Read the number of elements in the table ( this is because table entry has 5 values following it )*/
    regReadIntegerParameter(pAdapter, &STRCoexActivityNumOfElem,
                            COEX_ACTIVITY_TABLE_DEF_NUM, COEX_ACTIVITY_TABLE_MIN_NUM, COEX_ACTIVITY_TABLE_MAX_NUM,
                            sizeof p->twdInitParams.tGeneral.halCoexActivityTable.numOfElements, 
                            (TI_UINT8*)(&p->twdInitParams.tGeneral.halCoexActivityTable.numOfElements) );

    /* Read the CoexActivity table string */
    regReadStringParameter(pAdapter, &STRCoexActivityTable ,
                            (TI_INT8*)(defCoexActivityTable), strCoexActivitySize,
                            (TI_UINT8*)strCoexActivityTable, &strCoexActivitySize);

    /* Convert the CoexActivity table string */
    regConvertStringtoCoexActivityTable(strCoexActivityTable , p->twdInitParams.tGeneral.halCoexActivityTable.numOfElements, &p->twdInitParams.tGeneral.halCoexActivityTable.entry[0] , strCoexActivitySize);

	/*
    Power Manager
    */
    regReadIntegerParameter(pAdapter,
                            &STRPowerMode,
                            POWER_MODE_DEF_VALUE,
                            POWER_MODE_MIN_VALUE,
                            POWER_MODE_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.powerMode,
                            (TI_UINT8*)&p->PowerMgrInitParams.powerMode);

    regReadIntegerParameter(pAdapter,
                            &STRBeaconReceiveTime,
                            BEACON_RECEIVE_TIME_DEF_VALUE,
                            BEACON_RECEIVE_TIME_MIN_VALUE,
                            BEACON_RECEIVE_TIME_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.beaconReceiveTime,
                            (TI_UINT8*)&p->PowerMgrInitParams.beaconReceiveTime);

    regReadIntegerParameter(pAdapter,
                            &STRBaseBandWakeUpTime,
                            BASE_BAND_WAKE_UP_TIME_DEF_VALUE,
                            BASE_BAND_WAKE_UP_TIME_MIN_VALUE,
                            BASE_BAND_WAKE_UP_TIME_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.BaseBandWakeUpTime,
                            (TI_UINT8*)&p->PowerMgrInitParams.BaseBandWakeUpTime);

    regReadIntegerParameter(pAdapter,
                            &STRHangoverPeriod,
                            HANGOVER_PERIOD_DEF_VALUE,
                            HANGOVER_PERIOD_MIN_VALUE,
                            HANGOVER_PERIOD_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.hangoverPeriod,
                            (TI_UINT8*)&p->PowerMgrInitParams.hangoverPeriod);

    regReadIntegerParameter(pAdapter,
                            &STRBeaconListenInterval,
                            BEACON_LISTEN_INTERVAL_DEF_VALUE,
                            BEACON_LISTEN_INTERVAL_MIN_VALUE,
                            BEACON_LISTEN_INTERVAL_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.beaconListenInterval,
                            (TI_UINT8*)&p->PowerMgrInitParams.beaconListenInterval);

    regReadIntegerParameter(pAdapter,
                            &STRDtimListenInterval,
                            DTIM_LISTEN_INTERVAL_DEF_VALUE,
                            DTIM_LISTEN_INTERVAL_MIN_VALUE,
                            DTIM_LISTEN_INTERVAL_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.dtimListenInterval,
                            (TI_UINT8*)&p->PowerMgrInitParams.dtimListenInterval);

    regReadIntegerParameter(pAdapter,
                            &STRNConsecutiveBeaconsMissed,
                            N_CONSECUTIVE_BEACONS_MISSED_DEF_VALUE,
                            N_CONSECUTIVE_BEACONS_MISSED_MIN_VALUE,
                            N_CONSECUTIVE_BEACONS_MISSED_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.nConsecutiveBeaconsMissed,
                            (TI_UINT8*)&p->PowerMgrInitParams.nConsecutiveBeaconsMissed);

    regReadIntegerParameter(pAdapter,
                            &STREnterTo802_11PsRetries,
                            ENTER_TO_802_11_POWER_SAVE_RETRIES_DEF_VALUE,
                            ENTER_TO_802_11_POWER_SAVE_RETRIES_MIN_VALUE,
                            ENTER_TO_802_11_POWER_SAVE_RETRIES_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.EnterTo802_11PsRetries,
                            (TI_UINT8*)&p->PowerMgrInitParams.EnterTo802_11PsRetries);

    regReadIntegerParameter(pAdapter,
                            &STRAutoPowerModeInterval,
                            AUTO_POWER_MODE_INTERVAL_DEF_VALUE,
                            AUTO_POWER_MODE_INTERVAL_MIN_VALUE,
                            AUTO_POWER_MODE_INTERVAL_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.autoModeInterval,
                            (TI_UINT8*)&p->PowerMgrInitParams.autoModeInterval);

    regReadIntegerParameter(pAdapter,
                            &STRAutoPowerModeActiveTh,
                            AUTO_POWER_MODE_ACTIVE_TH_DEF_VALUE,
                            AUTO_POWER_MODE_ACTIVE_TH_MIN_VALUE,
                            AUTO_POWER_MODE_ACTIVE_TH_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.autoModeActiveTH,
                            (TI_UINT8*)&p->PowerMgrInitParams.autoModeActiveTH);

    regReadIntegerParameter(pAdapter,
                            &STRAutoPowerModeDozeTh,
                            AUTO_POWER_MODE_DOZE_TH_DEF_VALUE,
                            AUTO_POWER_MODE_DOZE_TH_MIN_VALUE,
                            AUTO_POWER_MODE_DOZE_TH_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.autoModeDozeTH,
                            (TI_UINT8*)&p->PowerMgrInitParams.autoModeDozeTH);

    regReadIntegerParameter(pAdapter,
                            &STRAutoPowerModeDozeMode,
                            AUTO_POWER_MODE_DOZE_MODE_DEF_VALUE,
                            AUTO_POWER_MODE_DOZE_MODE_MIN_VALUE,
                            AUTO_POWER_MODE_DOZE_MODE_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.autoModeDozeMode,
                            (TI_UINT8*)&p->PowerMgrInitParams.autoModeDozeMode);

    regReadIntegerParameter(pAdapter,
                            &STRDefaultPowerLevel,
                            POWERAUTHO_POLICY_ELP,
                            POWERAUTHO_POLICY_ELP,
                            POWERAUTHO_POLICY_AWAKE,
                            sizeof p->PowerMgrInitParams.defaultPowerLevel,
                            (TI_UINT8*)&p->PowerMgrInitParams.defaultPowerLevel);

    regReadIntegerParameter(pAdapter,
                            &STRPowerSavePowerLevel,
                            POWERAUTHO_POLICY_ELP,
                            POWERAUTHO_POLICY_ELP,
                            POWERAUTHO_POLICY_AWAKE,
                            sizeof p->PowerMgrInitParams.PowerSavePowerLevel,
                            (TI_UINT8*)&p->PowerMgrInitParams.PowerSavePowerLevel);


  regReadIntegerParameter(pAdapter,
                            &STRBurstModeEnable,
                            BURST_MODE_ENABLE_DEF,
                            BURST_MODE_ENABLE_MIN,
                            BURST_MODE_ENABLE_MAX,
                            sizeof p->qosMngrInitParams.bEnableBurstMode,
                            (TI_UINT8*)&p->qosMngrInitParams.bEnableBurstMode);


/*---------------------- Power Management Configuration -----------------------*/
    regReadIntegerParameter(pAdapter,
                            &STRPowerMgmtHangOverPeriod,
                            HANGOVER_PERIOD_DEF_VALUE,
                            HANGOVER_PERIOD_MIN_VALUE,
                            HANGOVER_PERIOD_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.hangOverPeriod,
                            (TI_UINT8*)&p->PowerMgrInitParams.hangOverPeriod);

    regReadIntegerParameter(pAdapter,
                            &STRPowerMgmtNeedToSendNullData,
                            POWER_MGMNT_NEED_TO_SEND_NULL_PACKET_DEF_VALUE,
                            POWER_MGMNT_NEED_TO_SEND_NULL_PACKET_MIN_VALUE,
                            POWER_MGMNT_NEED_TO_SEND_NULL_PACKET_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.needToSendNullData,
                            (TI_UINT8*)&p->PowerMgrInitParams.needToSendNullData);
    
    regReadIntegerParameter(pAdapter,
                            &STRPowerMgmtNullPktRateModulation,
                            POWER_MGMNT_NULL_PACKET_RATE_MOD_DEF_VALUE,
                            POWER_MGMNT_NULL_PACKET_RATE_MOD_MIN_VALUE,
                            POWER_MGMNT_NULL_PACKET_RATE_MOD_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.NullPktRateModulation,
                            (TI_UINT8*)&p->PowerMgrInitParams.NullPktRateModulation);
    
    regReadIntegerParameter(pAdapter,
                            &STRPowerMgmtNumNullPktRetries,
                            POWER_MGMNT_NUM_NULL_PACKET_RETRY_DEF_VALUE,
                            POWER_MGMNT_NUM_NULL_PACKET_RETRY_MIN_VALUE,
                            POWER_MGMNT_NUM_NULL_PACKET_RETRY_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.numNullPktRetries,
                            (TI_UINT8*)&p->PowerMgrInitParams.numNullPktRetries);

    regReadIntegerParameter(pAdapter,
                            &STRPowerMgmtPllLockTime,
                            PLL_LOCK_TIME_DEF_VALUE,
                            PLL_LOCK_TIME_MIN_VALUE,
                            PLL_LOCK_TIME_MAX_VALUE,
                            sizeof p->PowerMgrInitParams.PLLlockTime,
                            (TI_UINT8*)&p->PowerMgrInitParams.PLLlockTime);

    regReadIntegerParameter(pAdapter,
                            &STRPsPollDeliveryFailureRecoveryPeriod,
                            PS_POLL_FAILURE_PERIOD_DEF,
                            PS_POLL_FAILURE_PERIOD_MIN,
                            PS_POLL_FAILURE_PERIOD_MAX,
                            sizeof p->PowerMgrInitParams.PsPollDeliveryFailureRecoveryPeriod,
                            (TI_UINT8*)&p->PowerMgrInitParams.PsPollDeliveryFailureRecoveryPeriod);

    regReadIntegerParameter(pAdapter,
                            &STRHostClkSettlingTime,
                            HOST_CLK_SETTLE_TIME_USEC_DEF,
                            HOST_CLK_SETTLE_TIME_USEC_MIN,
                            HOST_CLK_SETTLE_TIME_USEC_MAX,
                            sizeof p->twdInitParams.tGeneral.uHostClkSettlingTime,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.uHostClkSettlingTime);

    regReadIntegerParameter(pAdapter,
                            &STRHostFastWakeupSupport,
                            HOST_FAST_WAKE_SUPPORT_DEF,
                            HOST_FAST_WAKE_SUPPORT_MIN,
                            HOST_FAST_WAKE_SUPPORT_MAX,
                            sizeof p->twdInitParams.tGeneral.uHostFastWakeupSupport,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.uHostFastWakeupSupport);
        
    /*--------------- Power Management Wake up conditions ------------------*/

    regReadIntegerParameter(pAdapter, &STRListenInterval,
                            TWD_LISTEN_INTERVAL_DEF, TWD_LISTEN_INTERVAL_MIN,
                            TWD_LISTEN_INTERVAL_MAX,
                            sizeof p->PowerMgrInitParams.listenInterval,
                            (TI_UINT8*)&p->PowerMgrInitParams.listenInterval);

    /*-----------------------------------------------------------------------*/

    /*--------------- Power Server Init Parameters ------------------*/
    regReadIntegerParameter(pAdapter,
                            &STRPowerMgmtNumNullPktRetries,
                            POWER_MGMNT_NUM_NULL_PACKET_RETRY_DEF_VALUE,
                            POWER_MGMNT_NUM_NULL_PACKET_RETRY_MIN_VALUE,
                            POWER_MGMNT_NUM_NULL_PACKET_RETRY_MAX_VALUE,
                            sizeof p->twdInitParams.tPowerSrv.numNullPktRetries,
                            (TI_UINT8*)&p->twdInitParams.tPowerSrv.numNullPktRetries);

        regReadIntegerParameter(pAdapter,
                            &STRPowerMgmtHangOverPeriod,
                            HANGOVER_PERIOD_DEF_VALUE,
                            HANGOVER_PERIOD_MIN_VALUE,
                            HANGOVER_PERIOD_MAX_VALUE,
                            sizeof p->twdInitParams.tPowerSrv.hangOverPeriod,
                            (TI_UINT8*)&p->twdInitParams.tPowerSrv.hangOverPeriod);
    /*-----------------------------------------------------------------------*/
    

    /* Scan SRV */
    regReadIntegerParameter(pAdapter, &STRNumberOfNoScanCompleteToRecovery,
                            SCAN_SRV_NUMBER_OF_NO_SCAN_COMPLETE_TO_RECOVERY_DEF,
                            SCAN_SRV_NUMBER_OF_NO_SCAN_COMPLETE_TO_RECOVERY_MIN,
                            SCAN_SRV_NUMBER_OF_NO_SCAN_COMPLETE_TO_RECOVERY_MAX,
                            sizeof (p->twdInitParams.tScanSrv.numberOfNoScanCompleteToRecovery),
                            (TI_UINT8*)&(p->twdInitParams.tScanSrv.numberOfNoScanCompleteToRecovery) );

        regReadIntegerParameter(pAdapter, &STRTriggeredScanTimeOut,
            SCAN_SRV_TRIGGERED_SCAN_TIME_OUT_DEF,
            SCAN_SRV_TRIGGERED_SCAN_TIME_OUT_MIN,
            SCAN_SRV_TRIGGERED_SCAN_TIME_OUT_MAX,
            sizeof (p->twdInitParams.tScanSrv.uTriggeredScanTimeOut),
            (TI_UINT8*)&(p->twdInitParams.tScanSrv.uTriggeredScanTimeOut) );


    /* Regulatory Domain */

    /* Indicate the time in which the STA didn't receive any country code and was not connected, and therefore
       will delete its current country code */
    regReadIntegerParameter(pAdapter, &STRTimeToResetCountryMs,
                        REGULATORY_DOMAIN_COUNTRY_TIME_RESET_DEF, REGULATORY_DOMAIN_COUNTRY_TIME_RESET_MIN,
                        REGULATORY_DOMAIN_COUNTRY_TIME_RESET_MAX, 
                        sizeof p->regulatoryDomainInitParams.uTimeOutToResetCountryMs,
                        (TI_UINT8*)&(p->regulatoryDomainInitParams.uTimeOutToResetCountryMs));

    /* 802.11d/h */
    regReadIntegerParameter(pAdapter, &STRMultiRegulatoryDomainEnabled,
                            MULTI_REGULATORY_DOMAIN_ENABLED_DEF, MULTI_REGULATORY_DOMAIN_ENABLED_MIN,
                            MULTI_REGULATORY_DOMAIN_ENABLED_MAX, 
                            sizeof p->regulatoryDomainInitParams.multiRegulatoryDomainEnabled,
                            (TI_UINT8*)&(p->regulatoryDomainInitParams.multiRegulatoryDomainEnabled));

    regReadIntegerParameter(pAdapter, &STRSpectrumManagementEnabled,
                            SPECTRUM_MANAGEMENT_ENABLED_DEF, SPECTRUM_MANAGEMENT_ENABLED_MIN,
                            SPECTRUM_MANAGEMENT_ENABLED_MAX, 
                            sizeof p->regulatoryDomainInitParams.spectrumManagementEnabled,
                            (TI_UINT8*)&(p->regulatoryDomainInitParams.spectrumManagementEnabled));

    regReadIntegerParameter(pAdapter, &STRSpectrumManagementEnabled,
                            SPECTRUM_MANAGEMENT_ENABLED_DEF, SPECTRUM_MANAGEMENT_ENABLED_MIN,
                            SPECTRUM_MANAGEMENT_ENABLED_MAX, 
                            sizeof p->SwitchChannelInitParams.dot11SpectrumManagementRequired,
                            (TI_UINT8*)&(p->SwitchChannelInitParams.dot11SpectrumManagementRequired));


    /* Scan Control Tables */
    regReadStringParameter(pAdapter, &STRScanControlTable24,
                           (TI_INT8*)&ScanControlTable24Def[0],(USHORT)(2 * NUM_OF_CHANNELS_24),
                            (TI_UINT8*)&(ScanControlTable24Tmp[0]),
                            (PUSHORT)&tableLen);

    for( loopIndex = tableLen ; loopIndex < 2 * NUM_OF_CHANNELS_24 ; loopIndex++)
        ScanControlTable24Tmp[loopIndex] = '0';

    decryptScanControlTable(ScanControlTable24Tmp,(TI_UINT8*)&(p->regulatoryDomainInitParams.desiredScanControlTable.ScanControlTable24.tableString[0]),2 * NUM_OF_CHANNELS_24);


    /* Scan Control Tables for 5 Ghz*/
    regReadStringParameter(pAdapter, &STRScanControlTable5,
                           (TI_INT8*)&ScanControlTable5Def[0],(USHORT)(2 * A_5G_BAND_NUM_CHANNELS),
                            (TI_UINT8*)&(ScanControlTable5Tmp[0]),
                            (PUSHORT)&tableLen);


    for( loopIndex = tableLen ; loopIndex < 2 * A_5G_BAND_NUM_CHANNELS ; loopIndex++)
        ScanControlTable5Tmp[loopIndex] = '0';

    decryptScanControlTable(ScanControlTable5Tmp,(TI_UINT8*)&(p->regulatoryDomainInitParams.desiredScanControlTable.ScanControlTable5.tableString[0]),2 * A_5G_BAND_NUM_CHANNELS);


    /* Tx Power */
    regReadIntegerParameter(pAdapter, &STRTxPower,
                            DEF_TX_POWER, MIN_TX_POWER, MAX_TX_POWER,
                            sizeof p->regulatoryDomainInitParams.desiredTxPower,
                            (TI_UINT8*)&p->regulatoryDomainInitParams.desiredTxPower);

    regReadIntegerParameter(pAdapter, &STRdot11WEPStatus,
                            RSN_WEP_STATUS_DEF, RSN_WEP_STATUS_MIN, RSN_WEP_STATUS_MAX,
                            sizeof p->rsnInitParams.privacyOn,
                            (TI_UINT8*)&p->rsnInitParams.privacyOn);
    /* reverse privacy value - windows is setting 1 as off */
    /*
        p->rsnInitParams.privacyMode = !(p->rsnInitParams.privacyOn);
        p->rsnInitParams.privacyOn = !(p->rsnInitParams.privacyOn);
    */

    regReadIntegerParameter(pAdapter, &STRdot11WEPDefaultKeyID,
                            RSN_DEFAULT_KEY_ID_DEF, RSN_DEFAULT_KEY_ID_MIN,
                            RSN_DEFAULT_KEY_ID_MAX,
                            sizeof p->rsnInitParams.defaultKeyId,
                            (TI_UINT8*)&p->rsnInitParams.defaultKeyId);


    regReadIntegerParameter(pAdapter, &STRMixedMode,
                            RSN_WEPMIXEDMODE_ENABLED_DEF, RSN_WEPMIXEDMODE_ENABLED_MIN,
                            RSN_WEPMIXEDMODE_ENABLED_MAX,
                            sizeof p->rsnInitParams.mixedMode,
                            (TI_UINT8*)&p->rsnInitParams.mixedMode);

    regReadIntegerParameter(pAdapter, &STRWPAMixedMode,
                            RSN_WPAMIXEDMODE_ENABLE_DEF, RSN_WPAMIXEDMODE_ENABLE_MIN, 
                            RSN_WPAMIXEDMODE_ENABLE_MAX,                         
                            sizeof p->rsnInitParams.WPAMixedModeEnable,
                            (TI_UINT8*)&p->rsnInitParams.WPAMixedModeEnable);         

    regReadIntegerParameter(pAdapter, &STRRSNPreAuth,
                            RSN_PREAUTH_ENABLE_DEF, RSN_PREAUTH_ENABLE_MIN,
                            RSN_PREAUTH_ENABLE_MAX,
                            sizeof p->rsnInitParams.preAuthSupport,
                            (TI_UINT8*)&p->rsnInitParams.preAuthSupport);

    regReadIntegerParameter(pAdapter, &STRRSNPreAuthTimeout,
                            RSN_PREAUTH_TIMEOUT_DEF, RSN_PREAUTH_TIMEOUT_MIN,
                            RSN_PREAUTH_TIMEOUT_MAX,
                            sizeof p->rsnInitParams.preAuthTimeout,
                            (TI_UINT8*)&p->rsnInitParams.preAuthTimeout);

	regReadIntegerParameter(pAdapter, &STRPairwiseMicFailureFilter,
                            PAIRWISE_MIC_FAIL_FILTER_DEF, PAIRWISE_MIC_FAIL_FILTER_MIN, 
							PAIRWISE_MIC_FAIL_FILTER_MAX,                         
                            sizeof p->rsnInitParams.bPairwiseMicFailureFilter, 
							(TI_UINT8*)&p->rsnInitParams.bPairwiseMicFailureFilter);         

    regReadWepKeyParameter(pAdapter, (TI_UINT8*)p->rsnInitParams.keys, p->rsnInitParams.defaultKeyId);


    /*---------------------------
            QOS parameters
    -----------------------------*/

    regReadIntegerParameter(pAdapter, &STRClsfr_Type,
                            CLSFR_TYPE_DEF, CLSFR_TYPE_MIN, 
                            CLSFR_TYPE_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.eClsfrType,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.eClsfrType); 

    switch(p->txDataInitParams.ClsfrInitParam.eClsfrType)
    {
        case D_TAG_CLSFR:
            /* Trivial mapping D-tag to D-tag - no need to read more keys*/
        break;

        case DSCP_CLSFR:

            regReadIntegerParameter(pAdapter, &STRNumOfCodePoints,
                            NUM_OF_CODE_POINTS_DEF, NUM_OF_CODE_POINTS_MIN, 
                            NUM_OF_CODE_POINTS_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.uNumActiveEntries,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.uNumActiveEntries);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier00_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier01_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier02_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier03_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier04_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier05_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier06_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier07_CodePoint,
                            DSCP_CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier08_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier09_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier10_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier11_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier12_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier13_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier14_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier15_CodePoint,
                            CLASSIFIER_CODE_POINT_DEF, CLASSIFIER_CODE_POINT_MIN, 
                            CLASSIFIER_CODE_POINT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].Dscp.CodePoint,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].Dscp.CodePoint);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier00_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier01_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier02_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier03_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier04_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier05_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier06_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier07_DTag,
                            DSCP_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier08_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier09_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier10_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier11_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier12_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier13_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier14_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].DTag);
            regReadIntegerParameter(pAdapter, &STRDSCPClassifier15_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].DTag);


        break;

        case PORT_CLSFR:

            regReadIntegerParameter(pAdapter, &STRNumOfDstPortClassifiers,
                            NUM_OF_PORT_CLASSIFIERS_DEF, NUM_OF_PORT_CLASSIFIERS_MIN, 
                            NUM_OF_PORT_CLASSIFIERS_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.uNumActiveEntries,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.uNumActiveEntries);
            regReadIntegerParameter(pAdapter, &STRPortClassifier00_Port,
                            PORT_CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier01_Port,
                            PORT_CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier02_Port,
                            PORT_CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier03_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier04_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier05_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier06_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier07_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier08_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier09_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier10_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier11_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier12_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier13_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier14_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier15_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].Dscp.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].Dscp.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRPortClassifier00_DTag,
                            PORT_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier01_DTag,
                            PORT_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier02_DTag,
                            PORT_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier03_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier04_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier05_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier06_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier07_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier08_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier09_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier10_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier11_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier12_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier13_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier14_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].DTag);
            regReadIntegerParameter(pAdapter, &STRPortClassifier15_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].DTag);
    
        break;


        case IPPORT_CLSFR: 

            regReadIntegerParameter(pAdapter, &STRNumOfDstIPPortClassifiers,
                            NUM_OF_IPPORT_CLASSIFIERS_DEF, NUM_OF_IPPORT_CLASSIFIERS_MIN, 
                            NUM_OF_IPPORT_CLASSIFIERS_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.uNumActiveEntries,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.uNumActiveEntries);

            regReadStringParameter(pAdapter, &STRIPPortClassifier00_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier01_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier02_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier03_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier04_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier05_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier06_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier07_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier08_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier09_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier10_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier11_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier12_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier13_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier14_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].Dscp.DstIPPort.DstIPAddress, 4);

            regReadStringParameter(pAdapter, &STRIPPortClassifier15_IPAddress, (TI_INT8*)(ClsfrIp), 11, (TI_UINT8*)ClsfrIpString, &ClsfrIpStringSize);
            initValusFromRgstryString( (TI_INT8*)(ClsfrIpString), (TI_INT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].Dscp.DstIPPort.DstIPAddress, 4);

            regReadIntegerParameter(pAdapter, &STRIPPortClassifier00_Port,
                            IPPORT_CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier01_Port,
                            IPPORT_CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier02_Port,
                            IPPORT_CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier03_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier04_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier05_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier06_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier07_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier08_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier09_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier10_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier11_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier12_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier13_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier14_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier15_Port,
                            CLASSIFIER_PORT_DEF, CLASSIFIER_PORT_MIN, 
                            CLASSIFIER_PORT_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].Dscp.DstIPPort.DstPortNum,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].Dscp.DstIPPort.DstPortNum);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier00_DTag,
                            IPPORT_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[0].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier01_DTag,
                            IPPORT_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[1].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier02_DTag,
                            IPPORT_CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[2].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier03_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[3].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier04_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[4].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier05_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[5].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier06_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[6].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier07_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[7].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier08_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[8].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier09_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[9].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier10_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[10].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier11_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[11].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier12_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[12].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier13_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[13].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier14_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[14].DTag);
            regReadIntegerParameter(pAdapter, &STRIPPortClassifier15_DTag,
                            CLASSIFIER_DTAG_DEF, CLASSIFIER_DTAG_MIN, 
                            CLASSIFIER_DTAG_MAX,                         
                            sizeof p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].DTag,
                            (TI_UINT8*)&p->txDataInitParams.ClsfrInitParam.ClsfrTable[15].DTag);
    
        break;

    }



  /* ---------------------------

       Traffic Intensity Threshold 

   ---------------------------*/
    regReadIntegerParameter(pAdapter, &STRTrafficIntensityThresHigh, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_HIGH_DEF, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_HIGH_MIN, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_HIGH_MAX, 
                            sizeof p->ctrlDataInitParams.ctrlDataTrafficThreshold.uHighThreshold, 
                            (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataTrafficThreshold.uHighThreshold);

    regReadIntegerParameter(pAdapter, &STRTrafficIntensityThresLow, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_LOW_DEF, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_LOW_MIN, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_LOW_MAX, 
                            sizeof p->ctrlDataInitParams.ctrlDataTrafficThreshold.uLowThreshold,
                            (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataTrafficThreshold.uLowThreshold);

    regReadIntegerParameter(pAdapter, &STRTrafficIntensityTestInterval, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_INTERVAL_DEF, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_INTERVAL_MIN, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_INTERVAL_MAX, 
                            sizeof p->ctrlDataInitParams.ctrlDataTrafficThreshold.TestInterval,
                            (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataTrafficThreshold.TestInterval);

    regReadIntegerParameter(pAdapter, &STRTrafficIntensityThresholdEnabled, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_ENABLED_DEF, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_ENABLED_MIN, 
                            CTRL_DATA_TRAFFIC_THRESHOLD_ENABLED_MAX, 
                            sizeof p->ctrlDataInitParams.ctrlDataTrafficThresholdEnabled,
                            (TI_UINT8*)&p->ctrlDataInitParams.ctrlDataTrafficThresholdEnabled);

    regReadIntegerParameter(pAdapter, &STRTrafficMonitorMinIntervalPercentage, 
                            TRAFFIC_MONITOR_MIN_INTERVAL_PERCENT_DEF, 
                            TRAFFIC_MONITOR_MIN_INTERVAL_PERCENT_MIN, 
                            TRAFFIC_MONITOR_MIN_INTERVAL_PERCENT_MAX, 
                            sizeof p->trafficMonitorMinIntervalPercentage,
                            (TI_UINT8*)&p->trafficMonitorMinIntervalPercentage);

    regReadIntegerParameter(pAdapter, &STRWMEEnable,
                            WME_ENABLED_DEF, WME_ENABLED_MIN,
                            WME_ENABLED_MAX,
                            sizeof p->qosMngrInitParams.wmeEnable,
                            (TI_UINT8*)&p->qosMngrInitParams.wmeEnable);

    regReadIntegerParameter(pAdapter, &STRTrafficAdmCtrlEnable,
                            QOS_TRAFFIC_ADM_CTRL_ENABLED_DEF, QOS_TRAFFIC_ADM_CTRL_ENABLED_MIN, 
                            QOS_TRAFFIC_ADM_CTRL_ENABLED_MAX,                         
                            sizeof p->qosMngrInitParams.trafficAdmCtrlEnable,
                            (TI_UINT8*)&p->qosMngrInitParams.trafficAdmCtrlEnable); 

    regReadIntegerParameter(pAdapter, &STRdesiredPsMode,
                            QOS_DESIRED_PS_MODE_DEF, QOS_DESIRED_PS_MODE_MIN, 
                            QOS_DESIRED_PS_MODE_MAX,                         
                            sizeof p->qosMngrInitParams.desiredPsMode,
                            (TI_UINT8*)&p->qosMngrInitParams.desiredPsMode); 

    regReadIntegerParameter(pAdapter, &STRQOSmsduLifeTimeBE,
                    QOS_MSDU_LIFE_TIME_BE_DEF, QOS_MSDU_LIFE_TIME_BE_MIN,
                    QOS_MSDU_LIFE_TIME_BE_MAX,
                    sizeof p->qosMngrInitParams.MsduLifeTime[QOS_AC_BE],
                    (TI_UINT8*)&p->qosMngrInitParams.MsduLifeTime[QOS_AC_BE]);

    regReadIntegerParameter(pAdapter, &STRQOSmsduLifeTimeBK,
                            QOS_MSDU_LIFE_TIME_BK_DEF, QOS_MSDU_LIFE_TIME_BK_MIN,
                            QOS_MSDU_LIFE_TIME_BK_MAX,
                            sizeof p->qosMngrInitParams.MsduLifeTime[QOS_AC_BK],
                            (TI_UINT8*)&p->qosMngrInitParams.MsduLifeTime[QOS_AC_BK]);

    regReadIntegerParameter(pAdapter, &STRQOSmsduLifeTimeVI,
                            QOS_MSDU_LIFE_TIME_VI_DEF, QOS_MSDU_LIFE_TIME_VI_MIN,
                            QOS_MSDU_LIFE_TIME_VI_MAX,
                            sizeof p->qosMngrInitParams.MsduLifeTime[QOS_AC_VI],
                            (TI_UINT8*)&p->qosMngrInitParams.MsduLifeTime[QOS_AC_VI]);

    regReadIntegerParameter(pAdapter, &STRQOSmsduLifeTimeVO,
                            QOS_MSDU_LIFE_TIME_VO_DEF, QOS_MSDU_LIFE_TIME_VO_MIN,
                            QOS_MSDU_LIFE_TIME_VO_MAX,
                            sizeof p->qosMngrInitParams.MsduLifeTime[QOS_AC_VO],
                            (TI_UINT8*)&p->qosMngrInitParams.MsduLifeTime[QOS_AC_VO]);


    regReadIntegerParameter(pAdapter, &STRQOSrxTimeOutPsPoll,
                    QOS_RX_TIMEOUT_PS_POLL_DEF, QOS_RX_TIMEOUT_PS_POLL_MIN,
                    QOS_RX_TIMEOUT_PS_POLL_MAX,
                    sizeof p->twdInitParams.tGeneral.rxTimeOut.psPoll,
                    (TI_UINT8*)&p->twdInitParams.tGeneral.rxTimeOut.psPoll);

    regReadIntegerParameter(pAdapter, &STRQOSrxTimeOutUPSD,
                    QOS_RX_TIMEOUT_UPSD_DEF, QOS_RX_TIMEOUT_UPSD_MIN,
                    QOS_RX_TIMEOUT_UPSD_MAX,
                    sizeof p->twdInitParams.tGeneral.rxTimeOut.UPSD,
                    (TI_UINT8*)&p->twdInitParams.tGeneral.rxTimeOut.UPSD);

    /* Note: The PsPoll wait timeout should be aligned with the UPSD setting */
   /* p->PowerMgrInitParams.HwPsPollResponseTimeout = (TI_UINT8)p->qosMngrInitParams.rxTimeout.UPSD;*/

    regReadIntegerParameter(pAdapter, &STRQOSwmePsModeBE,
                            QOS_WME_PS_MODE_BE_DEF, QOS_WME_PS_MODE_BE_MIN,
                            QOS_WME_PS_MODE_BE_MAX,
                            sizeof p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_BE],
                            (TI_UINT8*)&p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_BE]);

    regReadIntegerParameter(pAdapter, &STRQOSwmePsModeBK,
                            QOS_WME_PS_MODE_BK_DEF, QOS_WME_PS_MODE_BK_MIN,
                            QOS_WME_PS_MODE_BK_MAX,
                            sizeof p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_BK],
                            (TI_UINT8*)&p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_BK]);

    regReadIntegerParameter(pAdapter, &STRQOSwmePsModeVI,
                        QOS_WME_PS_MODE_VI_DEF, QOS_WME_PS_MODE_VI_MIN,
                        QOS_WME_PS_MODE_VI_MAX,
                        sizeof p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_VI],
                        (TI_UINT8*)&p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_VI]);

    regReadIntegerParameter(pAdapter, &STRQOSwmePsModeVO,
                        QOS_WME_PS_MODE_VO_DEF, QOS_WME_PS_MODE_VO_MIN,
                        QOS_WME_PS_MODE_VO_MAX,
                        sizeof p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_VO],
                        (TI_UINT8*)&p->qosMngrInitParams.desiredWmeAcPsMode[QOS_AC_VO]);

    regReadIntegerParameter(pAdapter, &STRQOStxBlksThresholdBE,
                            QOS_TX_BLKS_THRESHOLD_BE_DEF, QOS_TX_BLKS_THRESHOLD_MIN,
                            QOS_TX_BLKS_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_BE],
                            (TI_UINT8*)&p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_BE]);

    regReadIntegerParameter(pAdapter, &STRQOStxBlksThresholdBK,
                            QOS_TX_BLKS_THRESHOLD_BK_DEF, QOS_TX_BLKS_THRESHOLD_MIN,
                            QOS_TX_BLKS_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_BK],
                            (TI_UINT8*)&p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_BK]);
    
    regReadIntegerParameter(pAdapter, &STRQOStxBlksThresholdVI,
                            QOS_TX_BLKS_THRESHOLD_VI_DEF, QOS_TX_BLKS_THRESHOLD_MIN,
                            QOS_TX_BLKS_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_VI],
                            (TI_UINT8*)&p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_VI]);

    regReadIntegerParameter(pAdapter, &STRQOStxBlksThresholdVO,
                            QOS_TX_BLKS_THRESHOLD_VO_DEF, QOS_TX_BLKS_THRESHOLD_MIN,
                            QOS_TX_BLKS_THRESHOLD_MAX,
                            sizeof p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_VO],
                            (TI_UINT8*)&p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_VO]);

    regReadIntegerParameter(pAdapter, &STRWiFiMode,
                            WIFI_MODE_DEF, WIFI_MODE_MIN,
                            WIFI_MODE_MAX,
                            sizeof uWiFiMode,
                            (TI_UINT8*)&uWiFiMode);
    if (uWiFiMode)
    {
        /* special numbers for WiFi mode */
        p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_BE] = QOS_TX_BLKS_THRESHOLD_BE_DEF_WIFI_MODE;
        p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_BK] = QOS_TX_BLKS_THRESHOLD_BK_DEF_WIFI_MODE;
        p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_VI] = QOS_TX_BLKS_THRESHOLD_VI_DEF_WIFI_MODE;
        p->twdInitParams.tGeneral.TxBlocksThresholdPerAc[QOS_AC_VO] = QOS_TX_BLKS_THRESHOLD_VO_DEF_WIFI_MODE;

        p->qosMngrInitParams.MsduLifeTime[QOS_AC_BE] = QOS_MSDU_LIFE_TIME_BE_DEF_WIFI_MODE;
        p->qosMngrInitParams.MsduLifeTime[QOS_AC_BK] = QOS_MSDU_LIFE_TIME_BK_DEF_WIFI_MODE;
        p->qosMngrInitParams.MsduLifeTime[QOS_AC_VI] = QOS_MSDU_LIFE_TIME_VI_DEF_WIFI_MODE;
        p->qosMngrInitParams.MsduLifeTime[QOS_AC_VO] = QOS_MSDU_LIFE_TIME_VO_DEF_WIFI_MODE;
    }
    
    regReadIntegerParameter(pAdapter, &STRQOSShortRetryLimitBE,
                            QOS_SHORT_RETRY_LIMIT_BE_DEF, QOS_SHORT_RETRY_LIMIT_BE_MIN,
                            QOS_SHORT_RETRY_LIMIT_BE_MAX,
                            sizeof p->qosMngrInitParams.ShortRetryLimit[QOS_AC_BE],
                            (TI_UINT8*)&p->qosMngrInitParams.ShortRetryLimit[QOS_AC_BE]);

    regReadIntegerParameter(pAdapter, &STRQOSShortRetryLimitBK,
                            QOS_SHORT_RETRY_LIMIT_BK_DEF, QOS_SHORT_RETRY_LIMIT_BK_MIN,
                            QOS_SHORT_RETRY_LIMIT_BK_MAX,
                            sizeof p->qosMngrInitParams.ShortRetryLimit[QOS_AC_BK],
                            (TI_UINT8*)&p->qosMngrInitParams.ShortRetryLimit[QOS_AC_BK]);

    regReadIntegerParameter(pAdapter, &STRQOSShortRetryLimitVI,
                            QOS_SHORT_RETRY_LIMIT_VI_DEF, QOS_SHORT_RETRY_LIMIT_VI_MIN,
                            QOS_SHORT_RETRY_LIMIT_VI_MAX,
                            sizeof p->qosMngrInitParams.ShortRetryLimit[QOS_AC_VI],
                            (TI_UINT8*)&p->qosMngrInitParams.ShortRetryLimit[QOS_AC_VI]);

    regReadIntegerParameter(pAdapter, &STRQOSShortRetryLimitVO,
                            QOS_SHORT_RETRY_LIMIT_VO_DEF, QOS_SHORT_RETRY_LIMIT_VO_MIN,
                            QOS_SHORT_RETRY_LIMIT_VO_MAX,
                            sizeof p->qosMngrInitParams.ShortRetryLimit[QOS_AC_VO],
                            (TI_UINT8*)&p->qosMngrInitParams.ShortRetryLimit[QOS_AC_VO]);

    regReadIntegerParameter(pAdapter, &STRQOSLongRetryLimitBE,
                            QOS_LONG_RETRY_LIMIT_BE_DEF, QOS_LONG_RETRY_LIMIT_BE_MIN,
                            QOS_LONG_RETRY_LIMIT_BE_MAX,
                            sizeof p->qosMngrInitParams.LongRetryLimit[QOS_AC_BE],
                            (TI_UINT8*)&p->qosMngrInitParams.LongRetryLimit[QOS_AC_BE]);

    regReadIntegerParameter(pAdapter, &STRQOSLongRetryLimitBK,
                            QOS_LONG_RETRY_LIMIT_BK_DEF, QOS_LONG_RETRY_LIMIT_BK_MIN,
                            QOS_LONG_RETRY_LIMIT_BK_MAX,
                            sizeof p->qosMngrInitParams.LongRetryLimit[QOS_AC_BK],
                            (TI_UINT8*)&p->qosMngrInitParams.LongRetryLimit[QOS_AC_BK]);

    regReadIntegerParameter(pAdapter, &STRQOSLongRetryLimitVI,
                            QOS_LONG_RETRY_LIMIT_VI_DEF, QOS_LONG_RETRY_LIMIT_VI_MIN,
                            QOS_LONG_RETRY_LIMIT_VI_MAX,
                            sizeof p->qosMngrInitParams.LongRetryLimit[QOS_AC_VI],
                            (TI_UINT8*)&p->qosMngrInitParams.LongRetryLimit[QOS_AC_VI]);

    regReadIntegerParameter(pAdapter, &STRQOSLongRetryLimitVO,
                            QOS_LONG_RETRY_LIMIT_VO_DEF, QOS_LONG_RETRY_LIMIT_VO_MIN,
                            QOS_LONG_RETRY_LIMIT_VO_MAX,
                            sizeof p->qosMngrInitParams.LongRetryLimit[QOS_AC_VO],
                            (TI_UINT8*)&p->qosMngrInitParams.LongRetryLimit[QOS_AC_VO]);

    regReadIntegerParameter(pAdapter, &STRQOSAckPolicyBE,
                            QOS_ACK_POLICY_BE_DEF, QOS_ACK_POLICY_BE_MIN,
                            QOS_ACK_POLICY_BE_MAX,
                            sizeof p->qosMngrInitParams.acAckPolicy[QOS_AC_BE],
                            (TI_UINT8*)&p->qosMngrInitParams.acAckPolicy[QOS_AC_BE]);

    regReadIntegerParameter(pAdapter, &STRQOSAckPolicyBK,
                            QOS_ACK_POLICY_BK_DEF, QOS_ACK_POLICY_BK_MIN,
                            QOS_ACK_POLICY_BK_MAX,
                            sizeof p->qosMngrInitParams.acAckPolicy[QOS_AC_BK],
                            (TI_UINT8*)&p->qosMngrInitParams.acAckPolicy[QOS_AC_BK]);

    regReadIntegerParameter(pAdapter, &STRQOSAckPolicyVI,
                            QOS_ACK_POLICY_VI_DEF, QOS_ACK_POLICY_VI_MIN,
                            QOS_ACK_POLICY_VI_MAX,
                            sizeof p->qosMngrInitParams.acAckPolicy[QOS_AC_VI],
                            (TI_UINT8*)&p->qosMngrInitParams.acAckPolicy[QOS_AC_VI]);

    regReadIntegerParameter(pAdapter, &STRQOSAckPolicyVO,
                            QOS_ACK_POLICY_VO_DEF, QOS_ACK_POLICY_VO_MIN,
                            QOS_ACK_POLICY_VO_MAX,
                            sizeof p->qosMngrInitParams.acAckPolicy[QOS_AC_VO],
                            (TI_UINT8*)&p->qosMngrInitParams.acAckPolicy[QOS_AC_VO]);


    regReadIntegerParameter(pAdapter, &STRQoSqueue0OverFlowPolicy,
                    QOS_QUEUE_0_OVFLOW_POLICY_DEF, QOS_QUEUE_0_OVFLOW_POLICY_MIN,
                    QOS_QUEUE_0_OVFLOW_POLICY_MAX,
                    sizeof p->qosMngrInitParams.QueueOvFlowPolicy[0],
                    (TI_UINT8*)&p->qosMngrInitParams.QueueOvFlowPolicy[0]);
    
    regReadIntegerParameter(pAdapter, &STRQoSqueue1OverFlowPolicy,
                    QOS_QUEUE_1_OVFLOW_POLICY_DEF, QOS_QUEUE_1_OVFLOW_POLICY_MIN,
                    QOS_QUEUE_1_OVFLOW_POLICY_MAX,
                    sizeof p->qosMngrInitParams.QueueOvFlowPolicy[1],
                    (TI_UINT8*)&p->qosMngrInitParams.QueueOvFlowPolicy[1]);

    regReadIntegerParameter(pAdapter, &STRQoSqueue2OverFlowPolicy,
                    QOS_QUEUE_2_OVFLOW_POLICY_DEF, QOS_QUEUE_2_OVFLOW_POLICY_MIN,
                    QOS_QUEUE_2_OVFLOW_POLICY_MAX,
                    sizeof p->qosMngrInitParams.QueueOvFlowPolicy[2],
                    (TI_UINT8*)&p->qosMngrInitParams.QueueOvFlowPolicy[2]);

    regReadIntegerParameter(pAdapter, &STRQoSqueue3OverFlowPolicy,
                    QOS_QUEUE_3_OVFLOW_POLICY_DEF, QOS_QUEUE_3_OVFLOW_POLICY_MIN,
                    QOS_QUEUE_3_OVFLOW_POLICY_MAX,
                    sizeof p->qosMngrInitParams.QueueOvFlowPolicy[3],
                    (TI_UINT8*)&p->qosMngrInitParams.QueueOvFlowPolicy[3]);

    /* Packet Burst parameters    */

    regReadIntegerParameter(pAdapter, &STRQOSPacketBurstEnable,
                            QOS_PACKET_BURST_ENABLE_DEF, QOS_PACKET_BURST_ENABLE_MIN, 
                            QOS_PACKET_BURST_ENABLE_MAX,                         
                            sizeof p->qosMngrInitParams.PacketBurstEnable,
                            (TI_UINT8*)&p->qosMngrInitParams.PacketBurstEnable); 
    PRINTF(DBG_REGISTRY,( "STRQOSPacketBurstEnable = %d\n", p->qosMngrInitParams.PacketBurstEnable));
    regReadIntegerParameter(pAdapter, &STRQOSPacketBurstTxOpLimit,
                            QOS_PACKET_BURST_TXOP_LIMIT_DEF, QOS_PACKET_BURST_TXOP_LIMIT_MIN, 
                            QOS_PACKET_BURST_TXOP_LIMIT_MAX,                         
                            sizeof p->qosMngrInitParams.PacketBurstTxOpLimit,
                            (TI_UINT8*)&p->qosMngrInitParams.PacketBurstTxOpLimit); 



    /*---------------------------
        Measurement parameters
    -----------------------------*/

    regReadIntegerParameter(pAdapter, &STRMeasurTrafficThreshold,
                            MEASUREMENT_TRAFFIC_THRSHLD_DEF, MEASUREMENT_TRAFFIC_THRSHLD_MIN, MEASUREMENT_TRAFFIC_THRSHLD_MAX,
                            sizeof p->measurementInitParams.trafficIntensityThreshold,
                            (TI_UINT8*)&p->measurementInitParams.trafficIntensityThreshold);   

    regReadIntegerParameter(pAdapter, &STRMeasurMaxDurationOnNonServingChannel,
                            MEASUREMENT_MAX_DUR_NON_SRV_CHANNEL_DEF, MEASUREMENT_MAX_DUR_NON_SRV_CHANNEL_MIN, MEASUREMENT_MAX_DUR_NON_SRV_CHANNEL_MAX,
                            sizeof p->measurementInitParams.maxDurationOnNonServingChannel,
                            (TI_UINT8*)&p->measurementInitParams.maxDurationOnNonServingChannel);  


    /*---------------------------
          XCC Manager parameters
    -----------------------------*/
#ifdef XCC_MODULE_INCLUDED

    regReadIntegerParameter(pAdapter, &STRXCCModeEnabled,
                            XCC_MNGR_ENABLE_DEF, XCC_MNGR_ENABLE_MIN, XCC_MNGR_ENABLE_MAX,
                            sizeof p->XCCMngrParams.XCCEnabled,
                            (TI_UINT8*)&p->XCCMngrParams.XCCEnabled);


    p->measurementInitParams.XCCEnabled = p->XCCMngrParams.XCCEnabled;  

#endif

    regReadIntegerParameter(pAdapter, &STRXCCTestIgnoreDeAuth0,
                            XCC_TEST_IGNORE_DEAUTH_0_DEF, XCC_TEST_IGNORE_DEAUTH_0_MIN, XCC_TEST_IGNORE_DEAUTH_0_MAX,
                            sizeof p->apConnParams.ignoreDeauthReason0,
                            (TI_UINT8*)&p->apConnParams.ignoreDeauthReason0);
    
    /*---------------------------
      EEPROM less support
    -----------------------------*/
    regReadIntegerParameter(pAdapter, &STREEPROMlessModeSupported,
                            TWD_EEPROMLESS_ENABLE_DEF, TWD_EEPROMLESS_ENABLE_MIN,
                            TWD_EEPROMLESS_ENABLE_MAX,
                            sizeof p->twdInitParams.tGeneral.halCtrlEepromLessEnable,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.halCtrlEepromLessEnable);

    regReadStringParameter(pAdapter, &STRstationMacAddress,
                            (TI_INT8*)(defStaMacAddress0), 11,
                            (TI_UINT8*)staMACAddress, &regMACstrLen);
    
    /*reads the arp ip from table*/
    regReadStringParameter(pAdapter ,&STRArp_Ip_Addr,
                            (TI_INT8*)(defArpIpAddress),REG_ARP_IP_ADDR_STR_LEN,
                            (TI_UINT8*)staArpIpAddress,&regArpIpStrLen ) ;

    regReadIntegerParameter(pAdapter, &STRArp_Ip_Filter_Ena,
                            DEF_FILTER_ENABLE_VALUE, MIN_FILTER_ENABLE_VALUE, MAX_FILTER_ENABLE_VALUE,
                            sizeof p->twdInitParams.tArpIpFilter.isFilterEnabled,
                            (TI_UINT8*)&p->twdInitParams.tArpIpFilter.isFilterEnabled);


    initValusFromRgstryString( (TI_INT8*)(staArpIpAddress), (TI_INT8*)&p->twdInitParams.tArpIpFilter.addr, 4);

    
    initValusFromRgstryString( (TI_INT8*)(staMACAddress),
                                    (TI_INT8*)&(p->twdInitParams.tGeneral.StaMacAddress[0]),
                                    6);
/*fource FragThreshold to be even value (round it down)MR WLAN00003501*/
    p->twdInitParams.tGeneral.halCtrlFragThreshold &= 0xFFFE;





/*----------------------------------
    Health Monitor registry init
------------------------------------*/

    /* No scan complete recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledNoScanComplete,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ NO_SCAN_COMPLETE_FAILURE ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ NO_SCAN_COMPLETE_FAILURE ]) );
    
    /* Mailbox failure recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledMboxFailure,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ MBOX_FAILURE ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ MBOX_FAILURE ]) );

    /* HW awake failure recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledHwAwakeFailure,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ HW_AWAKE_FAILURE ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ HW_AWAKE_FAILURE ]) );
    
    /* TX stuck recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledTxStuck,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ TX_STUCK ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ TX_STUCK ]) );
    
    /* disconnect timeout recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledDisconnectTimeout,
                            0, 0, 1,   /* default is disabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ DISCONNECT_TIMEOUT ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ DISCONNECT_TIMEOUT ]) );
    
    /* Power save failure recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledPowerSaveFailure,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ POWER_SAVE_FAILURE ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ POWER_SAVE_FAILURE ]) );
    
    /* Measurement failure recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledMeasurementFailure,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ MEASUREMENT_FAILURE ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ MEASUREMENT_FAILURE ]) );

    /* Bus failure recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledBusFailure,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ BUS_FAILURE ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ BUS_FAILURE ]) );

    /* HW Watchdog Expired recovery enabled */
    regReadIntegerParameter(pAdapter, &STRRecoveryEnabledHwWdExpire,
                            1, 0, 1,   /* default is enabled */
                            sizeof (p->healthMonitorInitParams.recoveryTriggerEnabled[ HW_WD_EXPIRE ]),
                            (TI_UINT8*)&(p->healthMonitorInitParams.recoveryTriggerEnabled[ HW_WD_EXPIRE ]) );
    
/*-------------------------------------------
   RSSI/SNR Weights for Average calculations   
--------------------------------------------*/
    regReadIntegerParameter(pAdapter, &STRRssiBeaconAverageWeight,
                            TWD_RSSI_BEACON_WEIGHT_DEF, TWD_RSSI_BEACON_WEIGHT_MIN,
                            TWD_RSSI_BEACON_WEIGHT_MAX,
                            sizeof p->twdInitParams.tGeneral.uRssiBeaconAverageWeight, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.uRssiBeaconAverageWeight);

    regReadIntegerParameter(pAdapter, &STRRssiPacketAverageWeight,
                            TWD_RSSI_PACKET_WEIGHT_DEF, TWD_RSSI_PACKET_WEIGHT_MIN,
                            TWD_RSSI_PACKET_WEIGHT_MAX,
                            sizeof p->twdInitParams.tGeneral.uRssiPacketAverageWeight, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.uRssiPacketAverageWeight);

    regReadIntegerParameter(pAdapter, &STRSnrBeaconAverageWeight,
                            TWD_SNR_BEACON_WEIGHT_DEF, TWD_SNR_BEACON_WEIGHT_MIN,
                            TWD_SNR_BEACON_WEIGHT_MAX,
                            sizeof p->twdInitParams.tGeneral.uSnrBeaconAverageWeight, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.uSnrBeaconAverageWeight);

    regReadIntegerParameter(pAdapter, &STRSnrPacketAverageWeight,
                            TWD_SNR_PACKET_WEIGHT_DEF, TWD_SNR_PACKET_WEIGHT_MIN,
                            TWD_SNR_PACKET_WEIGHT_MAX,
                            sizeof p->twdInitParams.tGeneral.uSnrPacketAverageWeight, 
                            (TI_UINT8*)&p->twdInitParams.tGeneral.uSnrPacketAverageWeight);

/*----------------------------------
 TX power adjust
------------------------------------*/

    regReadIntegerParameter(pAdapter, &STRTxPowerCheckTime,
                            1, 1, 1200,  /* in units of 5000 ms */
                            sizeof p->siteMgrInitParams.TxPowerCheckTime,
                            (TI_UINT8*)&p->siteMgrInitParams.TxPowerCheckTime);


    regReadIntegerParameter(pAdapter, &STRTxPowerControlOn,
                            1, 0, 1,  /* on/off (1/0) default is on */
                            sizeof p->siteMgrInitParams.TxPowerControlOn,
                            (TI_UINT8*)&p->siteMgrInitParams.TxPowerControlOn);

    regReadIntegerParameter(pAdapter, &STRTxPowerRssiThresh,
                            38, 0, 200,  /* the value is positive and will be translated by driver */
                            sizeof p->siteMgrInitParams.TxPowerRssiThresh,
                            (TI_UINT8*)&p->siteMgrInitParams.TxPowerRssiThresh);

    regReadIntegerParameter(pAdapter, &STRTxPowerRssiRestoreThresh,
                            45, 0, 200,  /* the value is positive and will be translated by driver */
                            sizeof p->siteMgrInitParams.TxPowerRssiRestoreThresh,
                            (TI_UINT8*)&p->siteMgrInitParams.TxPowerRssiRestoreThresh);

    regReadIntegerParameter(pAdapter, &STRTxPowerTempRecover,
                            MIN_TX_POWER, MIN_TX_POWER, MAX_TX_POWER,  
                            sizeof p->regulatoryDomainInitParams.uTemporaryTxPower,
                            (TI_UINT8*)&p->regulatoryDomainInitParams.uTemporaryTxPower);

/*----------------------------------
 end of TX power adjust
------------------------------------*/

/*----------------------------------
 Scan Concentrator
------------------------------------*/
    regReadIntegerParameter (pAdapter, &STRMinimumDurationBetweenOsScans,
                             SCAN_CNCN_MIN_DURATION_FOR_OS_SCANS_DEF, SCAN_CNCN_MIN_DURATION_FOR_OS_SCANS_MIN, SCAN_CNCN_MIN_DURATION_FOR_OS_SCANS_MAX,
                             sizeof p->tScanCncnInitParams.uMinimumDurationBetweenOsScans,
                             (TI_UINT8*)&p->tScanCncnInitParams.uMinimumDurationBetweenOsScans);

    regReadIntegerParameter (pAdapter, &STRDfsPassiveDwellTimeMs,
                             SCAN_CNCN_DFS_PASSIVE_DWELL_TIME_DEF, SCAN_CNCN_DFS_PASSIVE_DWELL_TIME_MIN, SCAN_CNCN_DFS_PASSIVE_DWELL_TIME_MAX,
                             sizeof p->tScanCncnInitParams.uDfsPassiveDwellTimeMs,
                             (TI_UINT8*)&p->tScanCncnInitParams.uDfsPassiveDwellTimeMs);

    regReadIntegerParameter (pAdapter, &STRScanPushMode,
                             SCAN_CNCN_APP_PUSH_MODE_DEF, SCAN_CNCN_APP_PUSH_MODE_MIN, SCAN_CNCN_APP_PUSH_MODE_MAX,
                             sizeof p->tScanCncnInitParams.bPushMode,
                             (TI_UINT8*)&p->tScanCncnInitParams.bPushMode);

/*----------------------------------
 WSC 
------------------------------------*/
    regReadIntegerParameter( pAdapter, &STRParseWSCInBeacons,
                             WSC_PARSE_IN_BEACON_DEF, WSC_PARSE_IN_BEACON_MIN, WSC_PARSE_IN_BEACON_MAX,
                             sizeof p->tMlmeInitParams.parseWSCInBeacons,
                             (TI_UINT8*)&p->tMlmeInitParams.parseWSCInBeacons );

/*----------------------------------
 Current BSS
------------------------------------*/
    regReadIntegerParameter( pAdapter, &STRNullDataKeepAliveDefaultPeriod,
                             NULL_KL_PERIOD_DEF, NULL_KL_PERIOD_MIN, NULL_KL_PERIOD_MAX,
                             sizeof p->tCurrBssInitParams.uNullDataKeepAlivePeriod,
                             (TI_UINT8*)&p->tCurrBssInitParams.uNullDataKeepAlivePeriod );

/*----------------------------------
 Context Engine
------------------------------------*/
    regReadIntegerParameter( pAdapter, &STRContextSwitchRequired,
                             CONTEXT_SWITCH_REQUIRED_DEF, CONTEXT_SWITCH_REQUIRED_MIN, CONTEXT_SWITCH_REQUIRED_MAX,
                             sizeof p->tContextInitParams.bContextSwitchRequired,
                             (TI_UINT8*)&p->tContextInitParams.bContextSwitchRequired );

    /*
     *  set 802.11n init parameters
    */
    regReadIntegerParameter(pAdapter, &STR11nEnable,
                            HT_11N_ENABLED_DEF, HT_11N_ENABLED_MIN,
                            HT_11N_ENABLED_MAX,
                            sizeof p->twdInitParams.tGeneral.b11nEnable,
                            (TI_UINT8*)&p->twdInitParams.tGeneral.b11nEnable);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_0,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[0],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[0]);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_1,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[1],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[1]);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_2,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[2],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[2]);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_3,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[3],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[3]);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_4,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[4],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[4]);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_5,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[5],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[5]);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_6,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[6],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[6]);

    regReadIntegerParameter(pAdapter, &STRBaPolicyTid_7,
                            HT_BA_POLICY_DEF, HT_BA_POLICY_MIN,
                            HT_BA_POLICY_MAX,
                            sizeof p->qosMngrInitParams.aBaPolicy[7],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaPolicy[7]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_0,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[0],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[0]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_1,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[1],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[1]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_2,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[2],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[2]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_3,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[3],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[3]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_4,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[4],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[4]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_5,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[5],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[5]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_6,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[6],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[6]);

    regReadIntegerParameter(pAdapter, &STRBaInactivityTimeoutTid_7,
                            HT_BA_INACTIVITY_TIMEOUT_DEF, HT_BA_INACTIVITY_TIMEOUT_MIN,
                            HT_BA_INACTIVITY_TIMEOUT_MAX,
                            sizeof p->qosMngrInitParams.aBaInactivityTimeout[7],
                            (TI_UINT8*)&p->qosMngrInitParams.aBaInactivityTimeout[7]);

/*----------------------------------
 Radio module parameters
------------------------------------*/
regReadIntegerParameter(pAdapter, &STRTxBiPReferencePower_2_4G,
                        128, 0, 255,
                        sizeof (TI_INT8),
                        (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxBiPReferencePower_2_4G);

regReadIntegerTable (pAdapter, &STRTxBiPReferencePower_5G, RADIO_TX_BIP_REF_POWER_DEF_TABLE_5G,
                     NUMBER_OF_SUB_BANDS_IN_5G_BAND_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxBiPReferencePower_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8), TI_TRUE);

regReadIntegerParameter(pAdapter, &STRTxBiPOffsetdB_2_4G,
                        0,0,255,
                        sizeof (TI_UINT8),
                        (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxBiPOffsetdB_2_4G);

regReadIntegerTable (pAdapter, &STRTxBiPOffsetdB_5G, RADIO_TX_BIP_OFF_BD_5G,
                     NUMBER_OF_SUB_BANDS_IN_5G_BAND_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxBiPOffsetdB_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8), TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxPerRatePowerLimits_2_4G_Normal, RADIO_TX_PER_POWER_LIMITS_2_4_NORMAL_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPerRatePowerLimits_2_4G_Normal, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);


regReadIntegerTable (pAdapter, &STRTxPerRatePowerLimits_2_4G_Degraded, RADIO_TX_PER_POWER_LIMITS_2_4_DEGRADED_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPerRatePowerLimits_2_4G_Degraded, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxPerRatePowerLimits_5G_Normal, RADIO_TX_PER_POWER_LIMITS_5_NORMAL_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPerRatePowerLimits_5G_Normal, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxPerRatePowerLimits_5G_Degraded, RADIO_TX_PER_POWER_LIMITS_5_DEGRADED_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPerRatePowerLimits_5G_Degraded, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxPerChannelPowerLimits_2_4G_11b, RADIO_TX_PER_POWER_LIMITS_2_4_11B_DEF_TABLE,
                     NUMBER_OF_2_4_G_CHANNELS, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPerChannelPowerLimits_2_4G_11b, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxPerChannelPowerLimits_2_4G_OFDM, RADIO_TX_PER_POWER_LIMITS_2_4_OFDM_DEF_TABLE,
                     NUMBER_OF_2_4_G_CHANNELS, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPerChannelPowerLimits_2_4G_OFDM, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxPerChannelPowerLimits_5G_OFDM, RADIO_TX_PER_POWER_LIMITS_5_OFDM_DEF_TABLE,
                     NUMBER_OF_5G_CHANNELS, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPerChannelPowerLimits_5G_OFDM, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

/* in case of zero value, use this methode */
        RssiSize = tmpRssiTableSize*2 +tmpRssiTableSize - 1 ; /*includes spaces between bytes*/
  
    
        regReadStringParameter(pAdapter, &STRRxRssiAndProcessCompensation_2_4G ,
                            (TI_INT8*)(defRxRssiAndProcessCompensation_2_4G), RssiSize,
                            (TI_UINT8*)staRssiAndProcessCompensation, &RssiSize);

        regConvertStringtoBeaconIETable(staRssiAndProcessCompensation , 
                                        (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tStatRadioParams.RxRssiAndProcessCompensation_2_4G, 
                                        RSSI_AND_PROCESS_COMPENSATION_TABLE_SIZE);
 


regReadIntegerTable (pAdapter, &STRRxRssiAndProcessCompensation_5G, RADIO_RX_RSSI_PROCESS_5_DEF_TABLE,
                     RSSI_AND_PROCESS_COMPENSATION_TABLE_SIZE, NULL, (TI_INT8*)p->twdInitParams.tIniFileRadioParams.tStatRadioParams.RxRssiAndProcessCompensation_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);


regReadIntegerTable (pAdapter, &STRTxPDVsRateOffsets_2_4G, RADIO_TX_PD_VS_RATE_OFFSET_2_4_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPDVsRateOffsets_2_4G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxPDVsRateOffsets_5G, RADIO_TX_PD_VS_RATE_OFFSET_5_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxPDVsRateOffsets_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxIbiasTable_2_4G, RADIO_TX_BIAS_2_4_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxIbiasTable_2_4G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerTable (pAdapter, &STRTxIbiasTable_5G, RADIO_TX_BIAS_5_DEF_TABLE,
                     NUMBER_OF_RATE_GROUPS_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TxIbiasTable_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerParameter(pAdapter, &STRTXBiPReferencePDvoltage_2_4G,
                        RADIO_RX_FEM_INSERT_LOSS_2_4_DEF, RADIO_RX_FEM_INSERT_LOSS_2_4_MIN,
                        RADIO_RX_FEM_INSERT_LOSS_2_4_MAX,
                        sizeof (TI_UINT16),
                        (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TXBiPReferencePDvoltage_2_4G);

regReadIntegerTable (pAdapter, &STRTXBiPReferencePDvoltage_5G, RADIO_TX_BIP_REF_VOLTAGE_DEF_TABLE_5G,
                     NUMBER_OF_SUB_BANDS_IN_5G_BAND_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.TXBiPReferencePDvoltage_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT16), TI_TRUE);


regReadIntegerParameter(pAdapter, &STRRxFemInsertionLoss_2_4G,
                        14, 0, 255,
                        sizeof (TI_UINT8),
                        (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.RxFemInsertionLoss_2_4G);

regReadIntegerTable (pAdapter, &STRRxFemInsertionLoss_5G, RADIO_RX_FEM_INSERT_LOSS_5_DEF_TABLE,
                     NUMBER_OF_SUB_BANDS_IN_5G_BAND_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tDynRadioParams.RxFemInsertionLoss_5G,
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8), TI_TRUE);

regReadIntegerParameter(pAdapter, &STRRxTraceInsertionLoss_2_4G,
                        RADIO_RX_TRACE_INSERT_LOSS_2_4_DEF, RADIO_RX_TRACE_INSERT_LOSS_2_4_MIN,
                        RADIO_RX_TRACE_INSERT_LOSS_2_4_MAX,
                        sizeof (TI_UINT8), (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tStatRadioParams.RxTraceInsertionLoss_2_4G);

regReadIntegerParameter(pAdapter, &STRTXTraceLoss_2_4G,
                        RADIO_RX_TRACE_INSERT_LOSS_2_4_DEF, RADIO_RX_TRACE_INSERT_LOSS_2_4_MIN,
                        RADIO_RX_TRACE_INSERT_LOSS_2_4_MAX,
                        sizeof (TI_UINT8), (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tStatRadioParams.TXTraceLoss_2_4G);

regReadIntegerParameter(pAdapter, &STRRxTraceInsertionLoss_2_4G,
                        RADIO_RX_TRACE_INSERT_LOSS_2_4_DEF, RADIO_RX_TRACE_INSERT_LOSS_2_4_MIN,
                        RADIO_RX_TRACE_INSERT_LOSS_2_4_MAX,
                        sizeof (TI_UINT8), (TI_UINT8*)&p->twdInitParams.tIniFileRadioParams.tStatRadioParams.RxTraceInsertionLoss_2_4G);
            
regReadIntegerTable (pAdapter, &STRRxTraceInsertionLoss_5G, RADIO_RX_TRACE_INSERT_LOSS_5_DEF_TABLE,
                     NUMBER_OF_SUB_BANDS_IN_5G_BAND_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tStatRadioParams.RxTraceInsertionLoss_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8), TI_TRUE);

regReadIntegerTable (pAdapter, &STRTXTraceLoss_5G, RADIO_TX_TRACE_LOSS_5_DEF_TABLE,
                     NUMBER_OF_SUB_BANDS_IN_5G_BAND_E, NULL, (TI_INT8*)&p->twdInitParams.tIniFileRadioParams.tStatRadioParams.TXTraceLoss_5G, 
                     (TI_UINT32*)&uTempEntriesCount, sizeof (TI_UINT8),TI_TRUE);

regReadIntegerParameter(pAdapter, &STRFRefClock,
						RADIO_FREF_CLOCK_DEF, RADIO_FREF_CLOCK_MIN,
                        RADIO_FREF_CLOCK_MAX,
                        sizeof p->twdInitParams.tPlatformGenParams.RefClk,
                        (TI_UINT8*)&p->twdInitParams.tPlatformGenParams.RefClk);

regReadIntegerParameter(pAdapter, &STRFRefClockSettingTime,
						RADIO_FREF_CLOCK_SETTING_TIME_DEF, RADIO_FREF_CLOCK_SETTING_TIME_MIN,
                        RADIO_FREF_CLOCK_SETTING_TIME_MAX,
                        sizeof p->twdInitParams.tPlatformGenParams.SettlingTime,
                       (TI_UINT8*)&p->twdInitParams.tPlatformGenParams.SettlingTime);

regReadIntegerParameter(pAdapter, &STRTXBiPFEMAutoDetect,
						0,0,1,                             
                        sizeof p->twdInitParams.tPlatformGenParams.TXBiPFEMAutoDetect,
                        (TI_UINT8*)&p->twdInitParams.tPlatformGenParams.TXBiPFEMAutoDetect);

regReadIntegerParameter(pAdapter, &STRTXBiPFEMManufacturer,
						1,0,1,
                        sizeof p->twdInitParams.tPlatformGenParams.TXBiPFEMManufacturer,
                        (TI_UINT8*)&p->twdInitParams.tPlatformGenParams.TXBiPFEMManufacturer);

regReadIntegerParameter(pAdapter, &STRClockValidOnWakeup,
						0,0,1,
                        sizeof p->twdInitParams.tPlatformGenParams.ClockValidOnWakeup,
                        (TI_UINT8*)&p->twdInitParams.tPlatformGenParams.ClockValidOnWakeup);

regReadIntegerParameter(pAdapter, &STRDC2DCMode,
						0,0,1,
                        sizeof p->twdInitParams.tPlatformGenParams.DC2DCMode,
                        (TI_UINT8*)&p->twdInitParams.tPlatformGenParams.DC2DCMode);

regReadIntegerParameter(pAdapter, &STRSingle_Dual_Band_Solution,
						0,0,1,
                        sizeof p->twdInitParams.tPlatformGenParams.Single_Dual_Band_Solution,
                        (TI_UINT8*)&p->twdInitParams.tPlatformGenParams.Single_Dual_Band_Solution);


/*----------------------------------
 Driver-Main parameters
------------------------------------*/
    regReadIntegerParameter( pAdapter, &STRWlanDrvThreadPriority,
                             WLAN_DRV_THREAD_PRIORITY_DEF, WLAN_DRV_THREAD_PRIORITY_MIN, WLAN_DRV_THREAD_PRIORITY_MAX,
                             sizeof p->tDrvMainParams.uWlanDrvThreadPriority,
                             (TI_UINT8*)&p->tDrvMainParams.uWlanDrvThreadPriority);

    regReadIntegerParameter( pAdapter, &STRBusDrvThreadPriority,
                             BUS_DRV_THREAD_PRIORITY_DEF, BUS_DRV_THREAD_PRIORITY_MIN, BUS_DRV_THREAD_PRIORITY_MAX,
                             sizeof p->tDrvMainParams.uBusDrvThreadPriority,
                             (TI_UINT8*)&p->tDrvMainParams.uBusDrvThreadPriority);

    regReadIntegerParameter( pAdapter, &STRSdioBlkSizeShift,
                             SDIO_BLK_SIZE_SHIFT_DEF, SDIO_BLK_SIZE_SHIFT_MIN, SDIO_BLK_SIZE_SHIFT_MAX,
                             sizeof p->tDrvMainParams.uSdioBlkSizeShift,
                             (TI_UINT8*)&p->tDrvMainParams.uSdioBlkSizeShift);



#ifdef _WINDOWS
    /* set etherMaxPayloadSize parameter for MTU size setting */
   pAdapter->etherMaxPayloadSize = ETHER_MAX_PAYLOAD_SIZE;

#endif /* _WINDOWS */

}


/*-----------------------------------------------------------------------------

Routine Name:

    regReadIntegerParameter

Routine Description:


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
static void regReadIntegerParameter (
                 TWlanDrvIfObjPtr       pAdapter,
                 PNDIS_STRING           pParameterName,
                 TI_UINT32                  defaultValue,
                 TI_UINT32                  minValue,
                 TI_UINT32                  maxValue,
                 TI_UINT8                  parameterSize,
                 TI_UINT8*                 pParameter
                 )
{
    PNDIS_CONFIGURATION_PARAMETER   RetValue;
    NDIS_STATUS                     Status;
    TI_UINT32                           value;

    NdisReadConfiguration(&Status, &RetValue,
                          pAdapter->ConfigHandle, pParameterName,
                          NdisParameterInteger);

    if(Status != NDIS_STATUS_SUCCESS) {

        NdisReadConfiguration(&Status, &RetValue,
              pAdapter->ConfigHandle, pParameterName,
              NdisParameterString
              );

        if(Status == NDIS_STATUS_SUCCESS) {
            assignRegValue(&value, RetValue);
            RetValue->ParameterData.IntegerData = value;

        }

    }

    if (Status != NDIS_STATUS_SUCCESS ||
        RetValue->ParameterData.IntegerData < minValue ||
        RetValue->ParameterData.IntegerData > maxValue)
    {
        PRINTF(DBG_REGISTRY,( "NdisReadConfiguration fail\n"));
        value = defaultValue;

    } else
    {
        value = RetValue->ParameterData.IntegerData;
    }

    switch (parameterSize)
    {
    case 1:
        *((TI_UINT8*) pParameter) = (TI_UINT8) value;
        break;

    case 2:
        *((PUSHORT) pParameter) = (USHORT) value;
        break;

    case 4:
        *((TI_UINT32*) pParameter) = (TI_UINT32) value;
        break;

    default:
        PRINT(DBG_REGISTRY_ERROR, "TIWL: Illegal Registry parameter size\n");
        break;

    }

}

/*-----------------------------------------------------------------------------

Routine Name:

    regReadIntegerParameterHex

Routine Description:


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
static void regReadIntegerParameterHex (
                 TWlanDrvIfObjPtr       pAdapter,
                 PNDIS_STRING           pParameterName,
                 TI_UINT32              defaultValue,
                 TI_UINT32              minValue,
                 TI_UINT32              maxValue,
                 TI_UINT8               defaultSize,
                 TI_UINT8 *             pParameter)
{
    TI_UINT32   parameterSize = 0;
    TI_UINT32   value;
    TI_BOOL     paramFound;

    regReadStringParameter (pAdapter, 
                            pParameterName, 
                            "x", 
                            sizeof("x"),
                            pParameter,
                            &parameterSize);

    /* Note: the "x" is used as a dummy string to detect if the requested key 
             wasn't found (in that case the x is returned as sefault) */
    paramFound = os_memoryCompare(pAdapter, pParameter, "x", sizeof("x")) != 0;

    if (paramFound) 
    {
        value = tiwlnstrtoi_hex (pParameter, parameterSize);

        if (value < minValue || value > maxValue)
        {
            value = defaultValue;
        }
    }
    else 
    {
        value = defaultValue;
    }

    switch (defaultSize)
    {
    case 1:
        *((TI_UINT8*) pParameter) = (TI_UINT8) value;
        break;

    case 2:
        *((PUSHORT) pParameter) = (USHORT) value;
        break;

    case 4:
        *((TI_UINT32*) pParameter) = (TI_UINT32) value;
        break;

    default:
        PRINT(DBG_REGISTRY_ERROR, "TIWL: Illegal Registry parameter size\n");
        break;
    }
}

/*-----------------------------------------------------------------------------

Routine Name:

    regReadParameters

Routine Description:


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
static void regReadStringParameter (
                 TWlanDrvIfObjPtr       pAdapter,
                 PNDIS_STRING           pParameterName,
                 TI_INT8*                  pDefaultValue,
                 USHORT                 defaultLen,
                 TI_UINT8*                 pParameter,
                 void*                  pParameterSize
                 )
{
    PNDIS_CONFIGURATION_PARAMETER   RetValue;
    NDIS_STATUS                     Status;
    ANSI_STRING                     ansiString;
    TI_UINT8*                          pSizeChar = 0;
    PUSHORT                         pSizeShort = 0;

    if(defaultLen <= 256)
    {
        pSizeChar = (TI_UINT8*)pParameterSize;
        ansiString.MaximumLength = 256;
    }
    else
    {
        pSizeShort = (PUSHORT)pParameterSize;
        ansiString.MaximumLength = 32576;
    }

    NdisReadConfiguration(&Status, &RetValue,
                          pAdapter->ConfigHandle, pParameterName,
                          NdisParameterString);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        ansiString.Buffer = (TI_INT8*)pParameter;

        NdisUnicodeStringToAnsiString(&ansiString, &RetValue->ParameterData.StringData);
        if(defaultLen <= 256)
            *pSizeChar = (TI_UINT8)ansiString.Length;
        else
            *pSizeShort = (USHORT)ansiString.Length;
    } else
    {
        if(defaultLen <= 256)
            *pSizeChar = (TI_UINT8)defaultLen;
        else
            *pSizeShort = (USHORT)defaultLen;

        memcpy(pParameter, pDefaultValue, defaultLen);
    }

    PRINTF(DBG_REGISTRY_LOUD, ("Read String Registry:  %c%c%c%c%c%c%c%c%c%c%c%c = %s\n",
                          pParameterName->Buffer[0],
                          pParameterName->Buffer[1],
                          pParameterName->Buffer[2],
                          pParameterName->Buffer[3],
                          pParameterName->Buffer[4],
                          pParameterName->Buffer[5],
                          pParameterName->Buffer[6],
                          pParameterName->Buffer[7],
                          pParameterName->Buffer[8],
                          pParameterName->Buffer[9],
                          pParameterName->Buffer[10],
                          pParameterName->Buffer[11],
                          pParameter));

}



/*-----------------------------------------------------------------------------

Routine Name:

    regReadParameters

Routine Description:


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
static void regReadWepKeyParameter (TWlanDrvIfObjPtr pAdapter, TI_UINT8 *pKeysStructure, TI_UINT8 defaultKeyId)
{
    NDIS_STATUS                         status;
    TSecurityKeys                      *pSecKeys;
    int i;
    int len;
    TI_UINT8                               Buf[MAX_KEY_BUFFER_LEN];
    PNDIS_CONFIGURATION_PARAMETER       RetValue;
    ANSI_STRING                         ansiString;
    NDIS_STRING                         STRdot11DefaultWEPKey[4] =
                                                {   NDIS_STRING_CONST( "dot11WEPDefaultKey1" ),
                                                    NDIS_STRING_CONST( "dot11WEPDefaultKey2" ),
                                                    NDIS_STRING_CONST( "dot11WEPDefaultKey3" ),
                                                    NDIS_STRING_CONST( "dot11WEPDefaultKey4" )
                                                };



    PRINTF(DBG_REGISTRY_LOUD, ("Reading WEP keys\n"));

    pSecKeys = (TSecurityKeys*)pKeysStructure;

    /**/
    /* Read WEP from registry*/
    /**/
    for ( i = 0; i < DOT11_MAX_DEFAULT_WEP_KEYS; i++ )
    {
        NdisReadConfiguration(&status, &RetValue,
                              pAdapter->ConfigHandle, &STRdot11DefaultWEPKey[i],
                              NdisParameterString);

        if(status == NDIS_STATUS_SUCCESS)
        {
            ansiString.Buffer = (TI_INT8*)Buf;
            ansiString.MaximumLength = MAX_KEY_BUFFER_LEN;

            pSecKeys->keyIndex = i;
            pSecKeys->keyType = KEY_WEP;
            NdisZeroMemory((void *)pSecKeys->macAddress, 6);

            if(((char *)(RetValue->ParameterData.StringData.Buffer))[1] == 0)
            {
                NdisUnicodeStringToAnsiString(&ansiString, &RetValue->ParameterData.StringData);

                len = decryptWEP((TI_INT8*)Buf, (TI_INT8*)pSecKeys->encKey, ansiString.Length);
            } else {
                len = decryptWEP((TI_INT8*)RetValue->ParameterData.StringData.Buffer,
                                 (TI_INT8*)pSecKeys->encKey,
                                 RetValue->ParameterData.StringData.Length);
            }

            if(len < ACX_64BITS_WEP_KEY_LENGTH_BYTES)
            {
                PRINTF(DBG_REGISTRY_ERROR, ("Error: minimum WEP key size is 5 bytes(%d)\n", len));
                pSecKeys->keyType = KEY_NULL;
                len = 0;
            }
            else if(len < ACX_128BITS_WEP_KEY_LENGTH_BYTES)
            {
                len = ACX_64BITS_WEP_KEY_LENGTH_BYTES;
            }
            else if(len < ACX_256BITS_WEP_KEY_LENGTH_BYTES)
            {
                len = ACX_128BITS_WEP_KEY_LENGTH_BYTES;
            }
            else
                len = ACX_256BITS_WEP_KEY_LENGTH_BYTES;

            pSecKeys->encLen = (TI_UINT8)len;

        } 
        else
        {
            pSecKeys->keyType = KEY_NULL;
            pSecKeys->encLen = 0;
        }

#ifdef _WINDOWS
        /*create local keys cache*/
        pAdapter->DefaultWepKeys[i].KeyIndex = i;
        if(i==defaultKeyId)
            pAdapter->DefaultWepKeys[i].KeyIndex |= 0x80000000;
        pAdapter->DefaultWepKeys[i].KeyLength = pSecKeys->encLen;
        NdisMoveMemory((void *)pAdapter->DefaultWepKeys[i].KeyMaterial,
            (void *)pSecKeys->encKey, pSecKeys->encLen);
        pAdapter->DefaultWepKeys[i].Length = sizeof(OS_802_11_WEP);
#endif /* _WINDOWS */

        pSecKeys++;
    }
}

#define iswhite(c) ( (c==' ') || (c=='\t') || (c=='\n') )

/*
 *
 *       Fun:   isnumber
 *
 *       Desc:  check if the ascii character is a number in the given base
 *
 *       Ret:   1 if number is a digit, 0 if not.
 *
 *       Notes: none
 *
 *       File:  btoi.c
 *
 */

static TI_BOOL isnumber ( short *pi, char c, short base )
{

    /* return 1 if c is a digit in the give base, else return 0 */
    /* place value of digit at pi */
    if ( base == 16 )
    {
        if ( '0' <= c && c <= '9' )
        {
            *pi =  c - '0';
            return (1);
        }
        else if ( 'a' <= c && c <= 'f' )
        {
            *pi =  c - 'a' + 10 ;
            return (1);
        }
        else if ( 'A' <= c && c <= 'F' )
        {
            *pi =  c - 'A' + 10 ;
            return (1);
        }
        else
        {
            return (0);
        }
    }
    c -= '0';
    if ( 0 <= (signed char)c && c < base )
    {
        *pi =  c ;
        return (1);
    }
    else
    {
        return (0);
    }
} /* end of isnumber */


static short _btoi (char *sptr, short slen, int *pi, short base)
{
    char    *s, c ;
    short   d, sign ;
    int     result ;
    char    saved ;

    s           =  sptr ;
    result      =  0 ;
    saved       =  sptr [slen];
    sptr [slen] =  '\0';

    /* skip initial white space */
/*    while ( (c = *s++) && iswhite(c) ); */
    do
    {
       c = *s++;
       if (!(c  && iswhite(c))) 
         break;
    }while(1); 
  
    --s ;

    /* recognize optional sign */
    if ( *s == '-' )
    {
        sign =  - 1 ;
        s++ ;
    }
    else if ( *s == '+' )
    {
        sign =  1 ;
        s++ ;
    }
    else
    {
        sign =  1 ;
    }

    /* recognize optional hex# prefix */
    if ((base == 16) && ((*s == '0') && ((*(s + 1) == 'x') || (*(s + 1) == 'X'))
       ))
        s += 2 ;

    /* recognize digits */

/*    for (; (c = *s++) && isnumber(&d, c, base) ; )
    {
        result =  base * result + d ;
    }
*/    
    while(1)
    {
      c = *s++;
      if (!(c && isnumber(&d, c, base)))    
        break;
      result =  base * result + d ;
    };

    *pi         =  sign * result ;
    sptr [slen] =  saved ; /* restore character which we changed to null */
    return (s - sptr - 1);
} /* end of _btoi */

static int decryptWEP
(
  TI_INT8* pSrc,
  TI_INT8* pDst,
  TI_UINT32 len
)
{
  /**/
  /* key to use for encryption*/
  /**/
  static LPCSTR lpEncryptKey = "jkljz98c&2>a+t)cl5[d=n3;\"f_um6\\d~v%$HO1";
  int cnEncryptLen = strlen(lpEncryptKey);

  char cIn, cCrypt, cHex[3];
  int i, j, nLen;
  int nPos;

  nLen = len / 2;
  nPos = len;

  /* start reading from end*/
  nPos = len - 2;

  for(i = 0; (i < nLen) && (nPos >= 0); i++, nPos -= 2)
  {
    /* get hex character*/
    cHex[0] = pSrc[nPos];
    cHex[1] = pSrc[nPos + 1];
    cHex[2] = 0;

    _btoi ( cHex, 2, &j, 16);
    cIn = (char) j;

    cCrypt = lpEncryptKey[i % cnEncryptLen];
    cIn = cIn ^ cCrypt;

    pDst[i] = cIn;
  }

  PRINTF(DBG_REGISTRY_LOUD, ("First 5 bytes of WEP: %x-%x-%x-%x-%x\n",
    pDst[0],
    pDst[1],
    pDst[2],
    pDst[3],
    pDst[4]));

  return nLen;
}

static void initValusFromRgstryString
(
  TI_INT8* pSrc,
  TI_INT8* pDst,
  TI_UINT32 len
)
{
    int j;
    TI_UINT32 count;
    for (count = 0 ; count < len ; count++)
    {
        _btoi((char *) (pSrc+(count*3)),  2, &j, 16 );

        pDst[count] = (TI_UINT8) j;
    }
}

#ifdef TI_DBG

void regReadLastDbgState (TWlanDrvIfObjPtr pAdapter)
{
    NDIS_STRING OsDbgStr = NDIS_STRING_CONST("OsDbgState");
    PNDIS_CONFIGURATION_PARAMETER Value;
    NDIS_STATUS Status;

    NdisReadConfiguration(&Status, &Value,
                          pAdapter->ConfigHandle, &OsDbgStr,
                          NdisParameterInteger
                         );

    if (Status != NDIS_STATUS_SUCCESS)
    {

        TiDebugFlag = ((DBG_NDIS_OIDS | DBG_INIT | DBG_RECV | DBG_SEND | DBG_IOCTL | DBG_INTERRUPT) << 16) |
                DBG_SEV_VERY_LOUD | DBG_SEV_INFO | DBG_SEV_LOUD | DBG_SEV_ERROR | DBG_SEV_FATAL_ERROR;

    } else
    {

        PRINTF(DBG_REGISTRY_VERY_LOUD, ("TIWL: New Flag - 0x%X\n", Value->ParameterData.IntegerData));

        TiDebugFlag = Value->ParameterData.IntegerData;

    }
}

#endif  /* TI_DBG */



static void readRates(TWlanDrvIfObjPtr pAdapter, TInitTable *pInitTable)
{
    /*
    ** B band
    */
    regReadIntegerParameter(pAdapter, &STRdot11BasicRateMask_B,
                            BASIC_RATE_SET_1_2_5_5_11, BASIC_RATE_SET_1_2, BASIC_RATE_SET_1_2_5_5_11,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_B_MODE], 
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_B_MODE]);

    regReadIntegerParameter(pAdapter, &STRdot11SupportedRateMask_B,
                          SUPPORTED_RATE_SET_1_2_5_5_11_22, SUPPORTED_RATE_SET_1_2, SUPPORTED_RATE_SET_1_2_5_5_11_22,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_B_MODE], 
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_B_MODE]);
    /*
    ** G band (B&G rates)
    */
    regReadIntegerParameter(pAdapter, &STRdot11BasicRateMask_G,
                            BASIC_RATE_SET_1_2_5_5_11, BASIC_RATE_SET_1_2, BASIC_RATE_SET_1_2_5_5_11,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_G_MODE], 
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_G_MODE]);

    regReadIntegerParameter(pAdapter, &STRdot11SupportedRateMask_G,
                            SUPPORTED_RATE_SET_ALL, SUPPORTED_RATE_SET_1_2, SUPPORTED_RATE_SET_ALL,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_G_MODE], 
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_G_MODE]);

    /*
    ** A band
    */
    regReadIntegerParameter(pAdapter, &STRdot11BasicRateMask_A,
                            BASIC_RATE_SET_6_12_24, BASIC_RATE_SET_6_12_24, BASIC_RATE_SET_6_12_24,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_A_MODE], 
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_A_MODE]);

    regReadIntegerParameter(pAdapter, &STRdot11SupportedRateMask_A,
                            SUPPORTED_RATE_SET_UP_TO_54, SUPPORTED_RATE_SET_1_2, SUPPORTED_RATE_SET_UP_TO_54,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_A_MODE], 
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_A_MODE]);

    /*
    ** Dual band (A&G)
    */
    regReadIntegerParameter(pAdapter, &STRdot11BasicRateMask_AG,
                            BASIC_RATE_SET_1_2, BASIC_RATE_SET_1_2, BASIC_RATE_SET_1_2,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_DUAL_MODE],
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_DUAL_MODE]);

    regReadIntegerParameter(pAdapter, &STRdot11SupportedRateMask_AG,
                            SUPPORTED_RATE_SET_ALL_OFDM, SUPPORTED_RATE_SET_1_2, SUPPORTED_RATE_SET_ALL_OFDM,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_DUAL_MODE],
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_DUAL_MODE]);

    /*
    ** N supported
    */
    regReadIntegerParameter(pAdapter, &STRdot11BasicRateMask_N,
                            BASIC_RATE_SET_1_2_5_5_11, BASIC_RATE_SET_1_2, BASIC_RATE_SET_1_2_5_5_11,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_N_MODE],
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstryBasicRate[DOT11_N_MODE]);

    regReadIntegerParameter(pAdapter, &STRdot11SupportedRateMask_AG,
                            SUPPORTED_RATE_SET_ALL_MCS_RATES, SUPPORTED_RATE_SET_ALL_MCS_RATES, SUPPORTED_RATE_SET_ALL_MCS_RATES,
                            sizeof pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_N_MODE],
                            (TI_UINT8*)&pInitTable->siteMgrInitParams.siteMgrRegstrySuppRate[DOT11_N_MODE]);
}


static void decryptScanControlTable(TI_UINT8* src, TI_UINT8* dst, USHORT len)
{

    USHORT i;
    int parityFlag = 0;
    char tmp = 0;
    char finalChar = 0;

    for(i=0; i < len; i++)
    {
        switch(src[i])
        {
        case 'A':
        case 'a':
            tmp = 10;
            break;
        case 'B':
        case 'b':
            tmp = 11;
            break;
        case 'C':
        case 'c':
            tmp = 12;
            break;
        case 'D':
        case 'd':
            tmp = 13;
            break;
        case 'E':
        case 'e':
            tmp = 14;
            break;
        case 'F':
        case 'f':
            tmp = 15;
            break;
        default:
            if( (src[i] >='0') && (src[i] <= '9') )
                tmp = (src[i] - '0');
            else
                return; /* ERROR input char */
        }
        if(parityFlag == 0)
            finalChar =  tmp << 4;
        else
        {
            finalChar |= (tmp & 0x0f);
            dst[i/2] = finalChar;
        }
        parityFlag = 1-parityFlag;
    }
}


/*-----------------------------------------------------------------------------

Routine Name:

    regReadIntegerTable

Routine Description:
    reads any table format and insert it to another string.
    the delimiters of the tables can be:
     - space (" ")
     - comma (",")
    the table reads only integers thus its reads the following chars:
     - "0" till "9"
     - minus sign ("-")

Arguments:


Return Value:

    zero on success else - error number.

-----------------------------------------------------------------------------*/
TI_UINT32
regReadIntegerTable(
                     TWlanDrvIfObjPtr       pAdapter,
                     PNDIS_STRING           pParameterName,
                     TI_INT8*                  pDefaultValue,
                     USHORT                 defaultLen,
                     TI_UINT8*              pUnsignedParameter,
                     TI_INT8*               pSignedParameter,
                     TI_UINT32*             pEntriesNumber, /* returns the number of read entries */
                     TI_UINT8               uParameterSize,
                     TI_BOOL                bHex)
{
    TI_UINT32 parameterIndex = 0;
    int myNumber;

    TI_UINT32 index;
    TI_UINT32 bufferSize = 0;

    char tempBuffer[15];
    char *pTempBuffer = tempBuffer;
    TI_UINT32 tempBufferIndex = 0;

    TI_BOOL isDigit;
    TI_BOOL numberReady;
    TI_BOOL isSign;
    TI_BOOL    endOfLine;

    TI_UINT32 debugInfo = 0;

    CHAR Buffer[MAX_KEY_BUFFER_LEN];
    TI_INT8* pBuffer = (TI_INT8*)&Buffer;

    regReadStringParameter(pAdapter,
                           pParameterName,
                           pDefaultValue,
                           defaultLen,
                           (TI_UINT8*)pBuffer,
                           &bufferSize);

    index=0;
    do { /* Parsing one line */

        isSign = TI_FALSE;
        isDigit = TI_FALSE;
        numberReady = TI_FALSE;
        tempBufferIndex = 0;
        endOfLine = TI_FALSE;

        while ((numberReady==TI_FALSE) && (index<bufferSize))
        {
            /* Parsing one number */
            switch (pBuffer[index])
            {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    pTempBuffer[tempBufferIndex] = pBuffer[index];
                    ++tempBufferIndex;
                    isDigit = TI_TRUE;
                    break;
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    if (bHex)
                    {
                        pTempBuffer[tempBufferIndex] = pBuffer[index];
                        ++tempBufferIndex;
                        isDigit = TI_TRUE;
                    }
                    break;
                case '-':
                    pTempBuffer[tempBufferIndex] = pBuffer[index];
                    ++tempBufferIndex;
                    if (isDigit==TI_TRUE)
                    {
                        PRINTF(DBG_REGISTRY_INFO, ("Error in read parameter %c in line index %d\n\
                                               The sign '-' isn't in place!\n",pBuffer[index],index));
                        debugInfo = 1;
                    }
                    isSign = TI_TRUE;
                    break;

                case ' ':
                case '\t': /* tab char */
                    /* for space discard*/
                    if ((isDigit==TI_FALSE) && (isSign==TI_FALSE))
                    {
                        break;
                    }
                    /*
                    else we continue to the code of the case ','
                    */
                case '\0':
                case '\n':
                case '\r':
                    endOfLine = TI_TRUE;

                case ',':
                    /* end of number reading */
                    pTempBuffer[tempBufferIndex] = '\0';
                    if (isDigit == TI_FALSE)
                    {
                        PRINTF(DBG_REGISTRY_INFO, ("Error in end of number delimiter. number isn't ready.\
                            check index %d",index));
                        debugInfo = 2;
                    }
                    numberReady = TI_TRUE;
                    break;

                default:
                    PRINTF(DBG_REGISTRY_INFO, ("%s(%d) Error - unexpected parameter %c.\n",
                        __FILE__,__LINE__,pBuffer[index]));
                    debugInfo = 3;
                    break;
            }/* switch( pBuffer[index] ) */

            if (debugInfo != 0)
            {
                return debugInfo;
            }
            ++index;

        }/* while (numberReady==TI_FALSE)*/

        if (pTempBuffer[0] == '-')
        {
            ++pTempBuffer;
            if (!bHex)
            {
            myNumber = tiwlnstrtoi(pTempBuffer,tempBufferIndex-1);
            }
            else
            {
                myNumber = tiwlnstrtoi_hex(pTempBuffer,tempBufferIndex-1);
            }
            myNumber = -(myNumber);
        }
        else
        {
            if (!bHex)
            {
            myNumber = tiwlnstrtoi(pTempBuffer,tempBufferIndex);
        }
            else
            {
                myNumber = tiwlnstrtoi_hex(pTempBuffer,tempBufferIndex);
            }
        }

        switch (uParameterSize)
        {
        case 1:
            if (pUnsignedParameter)
            {
                ((TI_UINT8*) pUnsignedParameter)[parameterIndex] = (TI_UINT8) myNumber;
            }
            else
            {
                ((TI_INT8*) pSignedParameter)[parameterIndex] = (TI_INT8) myNumber;
            }
            break;

        case 2:
            if (pUnsignedParameter)
            {
                ((PUSHORT) pUnsignedParameter)[parameterIndex]  = (USHORT) myNumber;
            }
            else
            {
                ((TI_INT16*) pSignedParameter)[parameterIndex]  = (TI_INT16) myNumber;
            }
            break;

        case 4:
            if (pUnsignedParameter)
            {
                ((TI_UINT32*) pUnsignedParameter)[parameterIndex] = (TI_UINT32) myNumber;
            }
            else
            {
                ((TI_INT32*) pSignedParameter)[parameterIndex] = (TI_INT32) myNumber;
            }
            break;

        default:
            PRINTF(DBG_REGISTRY_INFO, ("%s(%d) Error - Illegal Registry parameter size.\n",
                __FILE__,__LINE__));
            break;
        }

        ++parameterIndex;
    
    }while ((index<bufferSize)&&(endOfLine==TI_FALSE));

    *pEntriesNumber = parameterIndex; /* return number of entries read */
    return debugInfo;
}


void assignRegValue(TI_UINT32* lValue, PNDIS_CONFIGURATION_PARAMETER ndisParameter)
{ 
    char b[8]; 
    ANSI_STRING a = {0, 0, 0};

    a.MaximumLength = sizeof(b);
    a.Buffer=(TI_INT8*)b;

    if(ndisParameter->ParameterData.StringData.Length <= sizeof (b) * 2) 
    { 
      if ( ((char *)(ndisParameter->ParameterData.StringData.Buffer))[1] == 0 )
      {
        NdisUnicodeStringToAnsiString ( &a, &(ndisParameter)->ParameterData.StringData );
        *lValue = tiwlnstrtoi ( (char *)a.Buffer, a.Length ); 
      } else { 
        *lValue = tiwlnstrtoi ( (char *)(ndisParameter->ParameterData.StringData.Buffer), ndisParameter->ParameterData.StringData.Length); 
      } 
    } else {
      *lValue = 0;
    }
  }

/*-----------------------------------------------------------------------------

Routine Name:

    regConvertStringtoBeaconIETable
    
Routine Description: Converts the Ip Adrress in a form of string readen from the Registry 
to the Ip Address Array to be stored in the init_table struct 


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
static void regConvertStringtoBeaconIETable(TI_UINT8 *staIpAddressString,TI_UINT8 *IpAddressArray, TI_UINT8 size)
{

char *ptr;
TI_UINT8 *tmpIpAddr;
TI_UINT8 value=0,value_l,value_h,add_value;
int i;


    /* Take the pointer to the string MAC Address to convert it to the Array MAC Address */
    ptr=(char *)staIpAddressString;
    tmpIpAddr = IpAddressArray;

    for(i=0 ; i<size ; ptr++)
    {
        
        /* The value can be or "0-9" or from "a-f" */
        value_l = (*ptr-'0');
        value_h = (*ptr - 'a');

        /*PRINTF(DBG_REGISTRY,("value_l [%d] value_h [%d] *ptr %c value %d\n",value_l,value_h,*ptr,value));*/

        if( (value_l <= 9) || (value_h <= 15 ) )
        {
            /* We are in an expected range */
            /* nCheck if 0-9 */
            if(value_l <= 9 )
            {
                add_value = value_l;
            }
            /* Check if a-f */
            else
            {
                /* 'a' is in fact 10 decimal in hexa */
                add_value = value_h + 10;
            }
            value = value*16 + add_value;               
        /*  PRINTF(DBG_REGISTRY,("value %d add_value %d  \n",value,add_value));*/


        }
        else
        {
            tmpIpAddr[i] = value;
        /*  PRINTF(DBG_REGISTRY,("tmpMacAddr[%d]  is %x\n",i,tmpMacAddr[i]));*/
            value = 0;
            i++;
        }

    }
}

/*-----------------------------------------------------------------------------

Routine Name:

    regConvertStringtoCoexActivityTable

Routine Description: Converts the CoexActivity string into CoexActivity config table 


Arguments:


Return Value:

    None

-----------------------------------------------------------------------------*/
static void regConvertStringtoCoexActivityTable(TI_UINT8 *strCoexActivityTable, TI_UINT32 numOfElements, TCoexActivity *CoexActivityArray, TI_UINT8 size)
{
    char *ptr;
    TI_UINT16 tmpArray[NUM_OF_COEX_ACTIVITY_PARAMS_IN_SG];
    TI_UINT16 value=0,value_l,value_h,add_value;
    TI_UINT32 i;
    int entry = 0;

    /* Take the pointer to the string MAC Address to convert it to the Array MAC Address */
    ptr=(char *)strCoexActivityTable;

    for (i=0 ;i < numOfElements*NUM_OF_COEX_ACTIVITY_PARAMS_IN_SG ;ptr++)
    {
        
        /* The value can be or "0-9" or from "a-f" */
        value_l = (*ptr - '0');
        value_h = (*ptr - 'a');

        /*PRINTF(DBG_REGISTRY,("value_l [%d] value_h [%d] *ptr %c value %d\n",value_l,value_h,*ptr,value));*/

        if( (value_l <= 9) || (value_h <= 15 ) )
        {
            /* We are in an expected range */
            /* nCheck if 0-9 */
            if(value_l <= 9 )
            {
                add_value = value_l;
            }
            /* Check if a-f */
            else
            {
                /* 'a' is in fact 10 decimal in hexa */
                add_value = value_h + 10;
            }
            value = value*16 + add_value;               
        /*  PRINTF(DBG_REGISTRY,("value %d add_value %d  \n",value,add_value));*/


        }
        else
        {
            tmpArray[i%NUM_OF_COEX_ACTIVITY_PARAMS_IN_SG] = value;
            value = 0;
            i++;
            if ((i%NUM_OF_COEX_ACTIVITY_PARAMS_IN_SG) == 0)
            {
                CoexActivityArray[entry].coexIp         = (TI_UINT8)tmpArray[0];
                CoexActivityArray[entry].activityId     = (TI_UINT8)tmpArray[1];
                CoexActivityArray[entry].defaultPriority= (TI_UINT8)tmpArray[2];
                CoexActivityArray[entry].raisedPriority = (TI_UINT8)tmpArray[3];
                CoexActivityArray[entry].minService     = tmpArray[4];
                CoexActivityArray[entry].maxService     = tmpArray[5];
                entry++;
            }
        }

    }
    
}

static void parse_hex_string(char * pString, TI_UINT8 StrLength, TI_UINT8 * pBuffer, TI_UINT8 * Length)
{
    char ch;
    int iter = 0;

    while ((iter < StrLength) && (ch = pString[iter]))
    {
        TI_UINT8 val = ((ch >= '0' && ch <= '9') ? (ch - '0') : 
                     (ch >= 'A' && ch <= 'F') ? (0xA + ch - 'A') :
                     (ch >= 'a' && ch <= 'f') ? (0xA + ch - 'a') : 0);

        /* even indexes go to the lower nibble, odd indexes push them to the */
        /* higher nibble and then go themselves to the lower nibble. */
        if (iter % 2)
            pBuffer[iter / 2] = ((pBuffer[iter / 2] << (BIT_TO_BYTE_FACTOR / 2)) | val);
        else
            pBuffer[iter / 2] = val;

        ++iter;
    }

    /* iter = 0 len = 0, iter = 1 len = 1, iter = 2 len = 1, and so on... */
    *Length = (iter + 1) / 2;
}

static void parse_binary_string(char * pString, TI_UINT8 StrLength, TI_UINT8 * pBuffer, TI_UINT8 * Length)
{
    char ch;
    int iter = 0;

    while ((iter < StrLength) && (ch = pString[iter]))
    {
        TI_UINT8 val = (ch == '1' ? 1 : 0);

        if (iter % BIT_TO_BYTE_FACTOR)
            pBuffer[iter / BIT_TO_BYTE_FACTOR] |= (val << (iter % BIT_TO_BYTE_FACTOR));
        else
            pBuffer[iter / BIT_TO_BYTE_FACTOR] = val;

        ++iter;
    }

    /* iter = 0 len = 0, iter = 1 len = 1, iter = 8 len = 1, and so on... */
    *Length = (iter + BIT_TO_BYTE_FACTOR - 1) / BIT_TO_BYTE_FACTOR;
}

static void parse_filter_request(TRxDataFilterRequest* request, TI_UINT8 offset, char * mask, TI_UINT8 maskLength, char * pattern, TI_UINT8 patternLength)
{
    request->offset = offset;
    request->maskLength = request->patternLength = 0;

    if (maskLength > 0)
    {
        parse_binary_string(mask, maskLength, request->mask, &request->maskLength);
        parse_hex_string(pattern, patternLength, request->pattern, &request->patternLength);
    }
}

