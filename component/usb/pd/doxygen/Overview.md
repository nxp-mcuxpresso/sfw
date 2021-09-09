@details USB has evolved from a data interface capable of supplying limited power to a primary provider of power with a data
interface. Today many devices charge or get their power from USB ports contained in laptops, cars, aircraft or even
wall sockets. USB has become a ubiquitous power socket for many small devices such as cell phones, MP3 players and
other hand-held devices. Users need USB to fulfill their requirements not only in terms of data but also to provide
power to, or charge their devices.
There are however, still many devices which either require an additional power connection to the wall, or exceed the
USB rated current in order to operate. Increasingly, international regulations require better energy management due
to ecological and practical concerns relating to the availability of power. Regulations limit the amount of power
available from the wall which has led to a pressing need to optimize power usage. The USB Power Delivery
Specification has the potential to minimize waste as it becomes a standard for charging devices.
@n This USB Type-C PD stack implements the Type-C spec and PD3.0 spec basic functions, such as Type-C connect/disconnect state machine, PD message function. The stack provides API interface and configuration way for user to initialize, user can configure the stack as self requirement.
The architecture and components of the USB Type-C PD stack are shown as below picture:
@image html pd_architecture.jpg
@image latex pd_architecture.jpg "pd architecture"
- Device Policy Manager: The device policy manager is the application's function. It implement the device policy manager function of PD spec. It manage power and negotiate the request (for example: decide accept pr_swap or reject it.)
- PD Stack: The PD stack implement the policy engine, protocol of PD spec and Type-c connect/disconnect state machine of Type-C spec.
- PHY interface and PHY driver: PHY interface is one common interface for different PHY, PD stack use this interface to operate the PHY. Different PHY implement the same interface, so one PD stack implementation can work with different PHY drivers.
- CMSIS I2C/SPI wrapper: This wrapper provide same API to PHY driver for CMSIS I2C and CMSIS SPI. PHY driver can call the same wrapper and don't need care much about the difference of I2C and SPI.
- PHY (PTN5110 or other PHY): the PHY IC.
- OSA: To support different RTOSes with the same code base, the OSA is used inside the PD stack to wrap the differences between RTOSes.
@n Note that the OSA is not supported for use in the PD application. Therefore, from the PD application’s view point, the OSA is invisible.
- One PD port instance contain one instance of the PD stack, one instance of the PHY driver, one instance of the CMSIS I2C/SPI wrapper and one PHY. In one system there can be many PD port instances. The device policy manager can contain many PD port instances.

@n Note: The interface between the PHY Driver and the PD stack is internal and is simplified in this document.


# USB PD Initialization flow {#USBPDInitFlow}
The PD stack initialization flow is as follow:
@image html pd_init_flow.jpg
@image latex pd_init_flow.jpg "pd init flow"
- PD_TimerIsrFunction and PD_xxxIsrFunction need be put in the corresponding ISR.
@n Note: xxx means PHY, for example: PD_PTN5110IsrFunction
- Initialize PD instance: call PD_InstanceInit to initialize PD instance, The callback function, configuration need be passed to this API. This API return one handle repensent the PD instance.
- The PD instance task is important, it accomplish the PD commands or control function. PD commands and control are introduced in other sections. 


# USB PD connect/disconnect flow {#USBPDConnectFlow}
The PD stack's connect/disconnect flow is as follow:
@image html pd_connect.jpg
@image latex pd_connect.jpg "pd connect"
- When connect, the PD_CONNECT event will callback to application. If the port is source, it should provide the vSafe5V to Vbus in this callback.
- Then the PD stack of source will send source_cap at the start-up of the state machine.
- When sink receive the source_cap, the PD_DPM_SNK_RECEIVE_PARTNER_SRC_CAP event will callback to application.
- Sink will callback PD_DPM_SNK_GET_RDO to get RDO from application, then sink start to request the power.
- Source will callback PD_DPM_SRC_RDO_REQUEST, application detemine to accept the request or reject.
- Source will call the power related callback function to provide the power.
- At last, the success or fail event will callback to application.
@n When disconnect, the PD_DISCONNECTED event will callback to application.


# USB PD control function {#USBPDControlFun}
The PD_Control API provide the control function, for example: get self power role. Please reference to the #PD_Control function description.


# USB PD common task {#USBPDOneTaskFun}
One feature is provided as PD_CONFIG_COMMON_TASK in the USB PD stack to reduce the RAM size consumption.
  - when PD_CONFIG_COMMON_TASK is enable, all the PD intances use one task. Application use the follow API to create task. This can reduce the RAM size reuirement.
    @n void PD_Task(void);
  - when PD_CONFIG_COMMON_TASK is disable, every PD intances use one task. Application use the follow API to create taskes for every instance and pass the PD instance handle to the API. This is more flexible, customer can configure different priority for different PD instance.
    @n void PD_InstanceTask(pd_handle pdHandle);

# USB PD alternate mode {#USBPDAltModeFun}
The PD stack alternate mode structure is as follow (The alternate mode is one part of PD stack):
@image html pd_altmode_structure.jpg
@image latex pd_altmode_structure.jpg "pd alternate mode"
- Initialize PD Alternate Mode
  @n There is one parameter (pd_instance_config_t *config) to initialize PD instance when calling PD_InstanceInit. PD alternate mode releated parameters are configured as follow in this parameter:
  @n PD alternate mode host configuration parameter.
  @image html pd_altmode_parameter.jpg
  @image latex pd_altmode_parameter.jpg "pd alternate mode host parameter"
  @n PD alternate mode slave configuration parameter.
  @image html pd_altmode_slave_parameter.jpg
  @image latex pd_altmode_slave_parameter.jpg "pd alternate mode slave parameter"
  - PD_CONFIG_ALT_MODE_SUPPORT configures alternate mode enable or not in usb_pd_config.h file.
  - If altModeConifg is NULL, the PD instance doesn't support alternate mode function; If altModeConfig is not NULL, the PD instance supports alternate mode function.
  - Alternate mode related parameters are defined by the strucutre pd_alt_mode_config_t.
  - pd_alt_mode_module_t defines the supported modules (DisplayPort, ThunderBolt etc).
  - pd_alt_mode_dp_host_config_t defines the DisplayPort host module parameter.
  - pd_alt_mode_dp_slave_config_t defines the DisplayPort slave module parameter.
  - For the shield host board, pd_dp_peripheral_config_t, pd_cbtl_crossbar_config_t, pd_ptn36502_config_t and pd_hpd_driver_config_t define the board peripherals' parameters to implement the DisplayPort function.
    pd_dp_peripheral_interface_t defines the function table that drives DP board related peripherals. pd_dp_peripheral_config_t defines the parameter for DP board related peripherals. The function table is as follow:
    - dpPeripheralInit: PD stack will call this function automatically and pass the boardChipConfig as parameter when initialize PD stack.
    - dpPeripheralDeinit: PD stack will call this function automatically when de-initialize PD stack.
    - dpPeripheralControl: implement the DP function.

- PD Alternate Mode Task
  @n Aternate mode application need create one task using the follow API.
  @n void PD_AltModeTask(void);
  - For BM, the API need be called periodically.
  - For FreeRTOS, application need create one task using the follow similar codes.
    @n void PD_PortAltModeTask(void *arg)
    {
        while (1)
        {
            PD_AltModeTask();
        }
    }


- PD Alternate Mode Run State
  @n PD alternate mode run as follow (take DisplayPort as example):
  - After device attach. Alternate mode will start the disvover identity/SVIDs sequence if data role is DFP.
  - Search modules configured by pd_alt_mode_module_t *modules. if there is one module's SVID is matched with the result of the discover SVIDs, enter next step; if there is no module matched, the steps are done.
  - Get the supported SVID's modes by discover modes.
  - If customer enables PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE, the modes will be passed to application, and application will detemine which mode is supported or there is no supported mode. If there is no supported mode and pin assign, the steps are done.
  - If customer doesn't enable PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE (PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE's default value is disabled), PD stack will detemine which mode to support by the configure parameter (uint8_t supportPinAssigns) as follow:
    - Get attached dvice supported pin assigns from the mode value. If there is supported pin assigns that matches supportPinAssigns parameter, enter next step. otherwise done.
    - If multiFunctionPrefered parameter is set, PD stack will prefer to select the pin assign (kPinAssign_B and kPinAssign_D);
    - If multiFunctionPrefered parameter is not set or the previous step doesn't select one pin assign, PD stack will prefer to select the 4 lane pin assignment (kPinAssign_C, kPinAssign_E and kPinAssign_A);
    - If previous step doesn't select one pin assign, PD stack will select the first supported pin assign configured by supportPinAssigns parameter.
    - If there is no selected pin assign, the steps are done.
  - PD stack will do the enter mode, status update and DP configure to enable the attached device's DisplayPort function.
- PD Alternate Mode Events
  @n PD alternate mode notify application through the PD intance callback. the callback is registered to PD stack by PD_InstanceInit API. There are follow three events currently:
  - PD_DPM_ALTMODE_DP_DFP_SELECT_MODE_AND_PINASSIGN: application need select the mode and pin assign in this callback, if PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE is enable. PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE is disable defaultly.
  - PD_DPM_ALTMODE_DP_DFP_MODE_CONFIGURED: DisplayPort alternate mode is entered and pin assign is configured, displayport video data can be transfered.
  - PD_DPM_ALTMODE_DP_DFP_MODE_UNCONFIGURED: DisplayPort's pin assign is configured as safe mode. DisplayPort video data cannot be transfered.

# USB PD auto policy {#USBPDAutoPolicyFun}
The PD stack supports auto policy function. It can be configured by #pd_auto_policy_t. PD_InstanceInit has one parameter called #pd_instance_config_t, pd_instance_config_t has one field called deviceConfig, this field's type is #pd_power_port_config_t, pd_power_port_config_t has one field called autoPolicyConfig, this field's type is #pd_auto_policy_t, it is used to configure the auto policy function.
| function | description |
| :------ | :------ |
| autoRequestPRSwapAsSource | It is valid when power role is source. @n-0: don't request power role swap automatically @n-1: request power role swap automatically when self is not external powered and partner is external powered, it only try one time, it will not retry if partner reply reject. |
| autoRequestPRSwapAsSink | It is valid when power role is sink. @n-0: don't request power role swap automatically @n-1: request power role swap automatically when self is external powered and partner is not external powered, it only try one time, it will not retry if partner reply reject. |
| autoAcceptPRSwapAsSource | It is valid when power role is source. @n-kAutoRequestProcess_NotSupport: this instance doesn't support this function @n-kAutoRequestProcess_Accept: accept pr_swap request if self is not external powered or partner is external powered @n-kAutoRequestProcess_Reject: reject pr_swap request |
| autoAcceptPRSwapAsSink | It is valid when power role is sink. @n-kAutoRequestProcess_NotSupport: this instance doesn't support this function @n-kAutoRequestProcess_Accept: accept pr_swap request if self is external powered or partner is not external powered @n-kAutoRequestProcess_Reject: reject pr_swap request|
| autoRequestDRSwap | It is valid when self is not in alternating mode and self is DRD. @n-kPD_DataRoleUFP: auto request swap to UFP when self is DFP, it only try one time, it will not retry if partner reply reject @n-kPD_DataRoleDFP: auto request swap to DFP when self is UFP, it only try one time, it will not retry if partner reply reject @n-kPD_DataRoleNone: this instance doesn't support this function |
| autoAcceptDRSwapToDFP | It is valid when data role is UFP. @n-kAutoRequestProcess_NotSupport: this instance doesn't support this function @n-kAutoRequestProcess_Accept: accept dr_swap request @n-kAutoRequestProcess_Reject: reject dr_swap request |
| autoAcceptDRSwapToUFP | It is valid when data role is DFP. @n-kAutoRequestProcess_NotSupport: this instance doesn't support this function @n-kAutoRequestProcess_Accept: accept dr_swap request @n-kAutoRequestProcess_Reject: reject dr_swap request |
| autoRequestVConnSwap | It is valid when self support Vconn. @n-kPD_NotVconnSource: auto request swap to turn off Vconn when self is Vconn source, it only try one time, it will not retry if partner reply reject @n-kPD_IsVconnSource: auto request swap to turn on Vconn when self is not Vconn source, it only try one time, it will not retry if partner reply reject @n-kPD_VconnNone: this instance doesn't support this function |
| autoAcceptVconnSwapToOn | It is valid when Vconn is off. @n-kAutoRequestProcess_NotSupport: this instance doesn't support this function @n-kAutoRequestProcess_Accept: accept vconn_swap request @n-kAutoRequestProcess_Reject: reject vconn_swap request |
| autoAcceptVconnSwapToOff | It is valid when Vconn is on. @n-kAutoRequestProcess_NotSupport: this instance doesn't support this function @n-kAutoRequestProcess_Accept: accept vconn_swap request @n-kAutoRequestProcess_Reject: reject vconn_swap request |
| autoSinkNegotiation | It is valid when power role is sink. @n-0: this instance doesn't support this function @n-1: calculate the highest power request based on self's sink capabilities and partner's source capabilities |


# USB PD command function {#USBPDCommandFun}
The PD_Command API provide the command that are defined in the PD3.0 spec, in the spec these command are called AMS.
For example: pr_swap AMS is called PD_DPM_CONTROL_PR_SWAP as PD_Command's parameter.
- PD_DPM_CONTROL_POWER_NEGOTIATION
  @n it is only used in source when source power change, the work flow is as follow:
  @image html power_negotiation1.jpg
  @image latex power_negotiation1.jpg "power negotiation 1"
  @image html power_negotiation2.jpg
  @image latex power_negotiation2.jpg "power negotiation 2"
  @image html power_negotiation3.jpg
  @image latex power_negotiation3.jpg "power negotiation 3"

- PD_DPM_CONTROL_REQUEST
  @n it is only used in sink, the work flow is as follow:
  @image html rdo_request1.jpg
  @image latex rdo_request1.jpg "rdo request 1"
  @image html rdo_request2.jpg
  @image latex rdo_request2.jpg "rdo request 2"
  @image html rdo_request3.jpg
  @image latex rdo_request3.jpg "rdo request 3"

- PD_DPM_CONTROL_GOTO_MIN
  @n goto min request, it is only used in source:
  @image html goto_min.jpg
  @image latex goto_min.jpg "goto min"

- PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES
  @n get partner source capabilities
  @image html get_partner_source_cap.jpg
  @image latex get_partner_source_cap.jpg "get partner source cap"

- PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES
  @n get partner sink capabilities：
  @image html get_partner_sink_cap.jpg
  @image latex get_partner_sink_cap.jpg "get partner sink cap"

- PD_DPM_CONTROL_PR_SWAP
  @n power role swap
  @image html pr_swap_source.jpg
  @image latex pr_swap_source.jpg "pr swap (source start)"
  @image html pr_swap_sink.jpg
  @image latex pr_swap_sink.jpg "pr swap (sink start)"

- PD_DPM_CONTROL_DR_SWAP
  @n data role swap
  @image html data_role_swap.jpg
  @image latex data_role_swap.jpg "data role swap"

- PD_DPM_CONTROL_VCONN_SWAP
  @n vconn role swap:
  @image html vconn_swap1.jpg
  @image latex vconn_swap1.jpg "vconn swap 1"
  @image html vconn_swap2.jpg
  @image latex vconn_swap2.jpg "vconn swap 2"

- PD_DPM_CONTROL_SOFT_RESET
  @n application can send soft_reset actively:
  @image html soft_reset_app.jpg
  @image latex soft_reset_app.jpg "soft reset (app start)"
  @n PD stack may send soft_reset when there is error.
  @image html soft_reset_stack.jpg
  @image latex soft_reset_stack.jpg "soft reset (stack start)"

- PD_DPM_CONTROL_HARD_RESET
  @n application can send hard_reset actively, and PD stack may send hard_reset when there is error.
  @image html hard_reset1.jpg
  @image latex hard_reset1.jpg "hard reset (source start)"
  @image html hard_reset2.jpg
  @image latex hard_reset2.jpg "hard reset (sink start)"

- PD_DPM_CONTROL_DISCOVERY_IDENTITY
  @n discovery identity
- PD_DPM_CONTROL_DISCOVERY_SVIDS
  @n discovery SVIDs
- PD_DPM_CONTROL_DISCOVERY_MODES
  @n discovery Modes
- PD_DPM_CONTROL_ENTER_MODE
  @n enter mode
- PD_DPM_CONTROL_EXIT_MODE
  @n exit mode
  @image html structured_vdm_reply.jpg
  @image latex structured_vdm_reply.jpg "standard structured vdm"
  Note: For exit_mode command, if VDMModeExitTimer time out, work as receiving busy.
- PD_DPM_CONTROL_SEND_ATTENTION
  @n attention
  @image html structured_vdm_noreply.jpg
  @image latex structured_vdm_noreply.jpg "attention"
- PD_DPM_SEND_VENDOR_STRUCTURED_VDM
  @n send vendor defined structured vdm
  @nIf the structured vdm has reply, the flow is as follow:
  @image html structured_vdm_reply.jpg
  @image latex structured_vdm_reply.jpg "vendor structured vdm (with reply)"
  @n If the structured vdm doesn't have reply, the flow is as follow:
  @image html structured_vdm_noreply.jpg
  @image latex structured_vdm_noreply.jpg "vendor structured vdm (without reply)"

- PD_DPM_SEND_UNSTRUCTURED_VDM
  @n send unstructured vdm
  @image html unstructured_vdm.jpg
  @image latex unstructured_vdm.jpg "unstructured vdm"

- PD_DPM_FAST_ROLE_SWAP
  @n fast role swap
  @image html fast_role_swap.jpg
  @image latex fast_role_swap.jpg "fast role swap"
  Note: the PD_SrcTurnOnTypeCVbus may not be called as the chart's flow.
  After receiving the fast role swap, new source need provide power to VBUS when VBUS <= vSafe5V.

- PD_DPM_GET_SRC_EXT_CAP
  @n get partner's source extended capabilities
  @image html get_source_cap_ext.jpg
  @image latex get_source_cap_ext.jpg "get source cap ext"

- PD_DPM_GET_STATUS
  @n get partner status.
  @image html get_status.jpg
  @image latex get_status.jpg "get status"

- PD_DPM_GET_BATTERY_CAP
  @n get partner battery cap
  @image html get_battery_cap.jpg
  @image latex get_battery_cap.jpg "get battery cap"

- PD_DPM_GET_BATTERY_STATUS
  @n get partner battery status
  @image html get_battery_status.jpg
  @image latex get_battery_status.jpg "get battery status"

- PD_DPM_GET_MANUFACTURER_INFO
  @n get partner's manufacturer info
  @image html get_manufacturer_info.jpg
  @image latex get_manufacturer_info.jpg "get manufacturer info"

- PD_DPM_ALERT
  @n alert
  @image html alert.jpg
  @image latex alert.jpg "alert"

- PD_DPM_CONTROL_CABLE_RESET
  @n cable reset
  @image html cable_reset.jpg
  @image latex cable_reset.jpg "cable reset"

# USB PD low power {#USBPDLowPowerFun}
The PD stack supports low power function. It can be configured by the macro of PD_CONFIG_PHY_LOW_POWER_LEVEL. The macro of PD_CONFIG_PHY_LOW_POWER_LEVEL can be configured as 0, 1 and 2. Users can choose different low power levels according to their own needs.
- 0, disable low power function. PTN5110 PHY will never enter low power state.
- 1, enter low power state when the port is in the detached state.
- 2, enter low power state when the port is in the detached state or there is not AMS in progress in the attached state.
When entering low power state, PTN5110 PHY will keep a low power consumption and meanwhile will disable the following functional blocks. For more information about the power consumption parameters, please refer to the PTN5110 datasheet.
- Disable VBUS detection
- Disable VBUS monitoring
- Disable VBUS voltage alarm
- Disable VBUS auto discharge
- Disable fast role swap
- Disable fault status reporting
- Enable I2C clock stretching
@n Note: The low power function is only for PTN5110 PHY. The low power function for MCU needs users to implement acorrding to their own needs.
