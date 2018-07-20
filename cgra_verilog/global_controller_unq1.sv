//
//--------------------------------------------------------------------------------
//          THIS FILE WAS AUTOMATICALLY GENERATED BY THE GENESIS2 ENGINE        
//  FOR MORE INFORMATION: OFER SHACHAM (CHIP GENESIS INC / STANFORD VLSI GROUP)
//    !! THIS VERSION OF GENESIS2 IS NOT FOR ANY COMMERCIAL USE !!
//     FOR COMMERCIAL LICENSE CONTACT SHACHAM@ALUMNI.STANFORD.EDU
//--------------------------------------------------------------------------------
//
//  
//	-----------------------------------------------
//	|            Genesis Release Info             |
//	|  $Change: 11904 $ --- $Date: 2013/08/03 $   |
//	-----------------------------------------------
//	
//
//  Source file: /horowitz/users/dhuff/CGRAGenerator/hardware/generator_z/global_controller/global_controller.svp
//  Source template: global_controller
//
// --------------- Begin Pre-Generation Parameters Status Report ---------------
//
//	From 'generate' statement (priority=5):
//
//		---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//
//	From Command Line input (priority=4):
//
//		---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//
//	From XML input (priority=3):
//
//		---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//
//	From Config File input (priority=2):
//
// ---------------- End Pre-Generation Pramameters Status Report ----------------

// cfg_bus_width (_GENESIS2_DECLARATION_PRIORITY_) = 32
//
// cfg_addr_width (_GENESIS2_DECLARATION_PRIORITY_) = 32
//
// cfg_op_width (_GENESIS2_DECLARATION_PRIORITY_) = 5
//
// IFC (_GENESIS2_DECLARATION_PRIORITY_) = 
//	InstancePath:top.global_controller.jtag_controller.ifc (template_ifc_unq1)
//
// num_analog_regs (_GENESIS2_DECLARATION_PRIORITY_) = 15
//
module global_controller_unq1 (
clk_in, reset_in,
analog_r0,
analog_r1,
analog_r2,
analog_r3,
analog_r4,
analog_r5,
analog_r6,
analog_r7,
analog_r8,
analog_r9,
analog_r10,
analog_r11,
analog_r12,
analog_r13,
analog_r14,
config_data_in,
config_addr_out,
config_data_out,
clk_out,
reset_out,
cgra_stalled,
read,
write,
tdi,
tdo,
tms,
tck,
trst_n,
//FROM ANALOG
jm_out
);
  
  input  tck;
  input  clk_in;
  input  reset_in;
  input  tdi;
  input  tms;
  input  trst_n;
  input [19:0] jm_out;
  input [31:0] config_data_in;

  output reg read;
  output reg write;
  output reg [31:0] config_addr_out;
  output reg [31:0] config_data_out;
  output reg clk_out;
  output reg reset_out;
  output reg [3:0] cgra_stalled;
  output tdo;
  
  //value of each register in analog regfile
  output [31:0]  analog_r0;
  output [31:0]  analog_r1;
  output [31:0]  analog_r2;
  output [31:0]  analog_r3;
  output [31:0]  analog_r4;
  output [31:0]  analog_r5;
  output [31:0]  analog_r6;
  output [31:0]  analog_r7;
  output [31:0]  analog_r8;
  output [31:0]  analog_r9;
  output [31:0]  analog_r10;
  output [31:0]  analog_r11;
  output [31:0]  analog_r12;
  output [31:0]  analog_r13;
  output [31:0]  analog_r14;
	

  wire [31:0] config_addr_jtag_out;
  wire [31:0] config_data_jtag_out;
  wire [4:0] op;
  reg [31:0] config_data_jtag_in;
  reg clk;
  reg clk_domain;
  reg sys_clk_activated;
  //Extra flops to cross clock boundary
  reg sys_clk_act_sync_1;
  reg sys_clk_act_sync_2;
  //separate op out from address field
template_ifc_unq1  jtag_ifc (.Clk(clk), .Reset(reset_in));
jtag_unq1  jtag_controller (.ifc(jtag_ifc),
			    .sys_clk_activated(sys_clk_activated),
			    .bsr_tdi(),
			    .bsr_sample(),
			    .bsr_intest(),
			    .bsr_extest(),
			    .bsr_update_en(),
			    .bsr_capture_en(),
		 	    .bsr_shift_dr(),
			    .bsr_tdo()
			    );

//Connect signals to jtag_ifc ports
      assign jtag_ifc.tck=tck;
      assign jtag_ifc.tdi=tdi;
      assign jtag_ifc.tms=tms;
      assign jtag_ifc.trst_n=trst_n;
      assign jtag_ifc.config_data_from_gc=config_data_jtag_in; 
      assign tdo = jtag_ifc.tdo;
      assign config_data_jtag_out = jtag_ifc.config_data_to_gc;
      assign config_addr_jtag_out = jtag_ifc.config_addr_to_gc;
      assign op = jtag_ifc.config_op_to_gc;


// Internal signals to/from analog registers
reg analog_wr_en; 
reg [31:0] analog_data_in;
reg [31:0] analog_addr;
// instantiate registers for analog stuff
 analog_regfile  analog_regfile (
	.wr_en(analog_wr_en),
	.data_in(analog_data_in),
	.addr(analog_addr),
	.r0(analog_r0),
	.r1(analog_r1),
	.r2(analog_r2),
	.r3(analog_r3),
	.r4(analog_r4),
	.r5(analog_r5),
	.r6(analog_r6),
	.r7(analog_r7),
	.r8(analog_r8),
	.r9(analog_r9),
	.r10(analog_r10),
	.r11(analog_r11),
	.r12(analog_r12),
	.r13(analog_r13),
	.r14(analog_r14),
	.jm_out(jm_out),
	.clk(clk),
	.reset(reset_out)
	);

  //OPCODES
  parameter NOP = 5'd0;
  parameter write_config =5'd1;
  parameter read_config = 5'd2;
  //parameter set_base_reg = 5'd3;
  //parameter set_rw_and_count = 5'd4;
  //parameter burst_start = 5'd5;
  //parameter write_data = 5'd5;
  //parameter read_data = 5'd6;
  parameter write_A050 = 5'd7;
  parameter write_TST = 5'd8;
  parameter read_TST = 5'd9;
  parameter global_reset = 5'd10;
  //parameter reset_tile = 5'd11; 
  parameter write_stall = 5'd11;
  parameter read_stall = 5'd12;
  parameter advance_clk = 5'd13;
  //parameter resume_clk = 5'd14;
  parameter read_clk_domain = 5'd14;
  parameter switch_clk = 5'd15;
  parameter wr_rd_delay_reg = 5'd16;
  parameter rd_rd_delay_reg = 5'd17;
  parameter wr_delay_sel_reg = 5'd18;
  parameter rd_delay_sel_reg = 5'd19;
  parameter wr_analog_reg = 5'd20;
  parameter rd_analog_reg = 5'd21;
  //STATES FOR IGNORING INCOMING INSTRUCTIONS
  parameter ready = 3'd0;
  parameter reading = 3'd1;
  parameter resetting = 3'd2;
  parameter advancing_clk = 3'd3;
  parameter switching_clk = 3'd4;
  reg [2:0] state;
  //USED to remember old stall signal when advancing clk
  reg [3:0] stall_restore;
 
  logic all_stalled_tck;
  logic all_stalled_tck_rise;
  logic all_stalled_tck_fall;

  logic all_stalled_sys;
  logic all_stalled_sys_rise;
  logic all_stalled_sys_fall;

  //delay_sel[1] specifies delay for all_stalled_tck
  //delay_sel[0] specifices delay for all_stalled_sys
  logic [1:0] delay_sel;

  logic clk_switch_request;
  //clk_switch_request has to cross clk domains. We'll need sync FFs
  //for crossing from CLK -> TCK
  logic clk_switch_request_sync_1_tck;
  logic clk_switch_request_sync_2_tck;
  //for crossing from CLK -> SYS_CLK
  logic clk_switch_request_sync_1_sys;
  logic clk_switch_request_sync_2_sys;
  
  logic [31:0] TST;
  logic [31:0] counter;
  logic [31:0] rd_delay_reg;
  logic [4:0] clk_switch_counter_tck;
  always_comb begin
    	clk_domain = (sys_clk_activated) ? clk_in : tck;//Are we running the GC/CGRA on sys clk or test clk?
	all_stalled_tck = delay_sel[1] ? all_stalled_tck_fall : all_stalled_tck_rise;
	all_stalled_sys = delay_sel[0] ? all_stalled_sys_fall : all_stalled_sys_rise;
	clk = ((all_stalled_tck & !sys_clk_activated) | (all_stalled_sys & sys_clk_activated )) ? 0 : clk_domain; //Are we stalling both GC and CGRA during clk domain switch?
	clk_out = clk;
   	reset_out = (state==resetting) ? 1 : reset_in;
  end
  //Clock switch counter block (Always on test_clk)
  always @ (posedge tck or posedge reset_in) begin
	if (reset_in==1) begin
		clk_switch_counter_tck <= 5'h1F;
		sys_clk_activated <= 0;
		all_stalled_tck_rise <= 0;
	end
	else if ((clk_switch_request_sync_2_tck != sys_clk_activated) & (clk_switch_counter_tck > 0)) begin
		all_stalled_tck_rise <= 1;//Deactivate clk to rest of GC
		clk_switch_counter_tck <= clk_switch_counter_tck - 1;	
	end
	else if ((clk_switch_request_sync_2_tck != sys_clk_activated) & clk_switch_counter_tck <= 0 ) begin
		sys_clk_activated <= clk_switch_request;
		clk_switch_counter_tck <= clk_switch_counter_tck-1;
	end
	else if ((clk_switch_request_sync_2_tck == 0) && (sys_clk_activated == 0)) begin
		all_stalled_tck_rise <= 0;
	end	
  end
	

  //FALLING EDGE TCK. In case we need to delay the clock gating signal by an extra half cycle
  always @ (negedge tck or posedge reset_in) begin
	if (reset_in==1)
		all_stalled_tck_fall <= 0;
	else
		all_stalled_tck_fall <= all_stalled_tck_rise;
  end
  
  //IMPLEMENT 2 SYNC FLOPS TO CROSS CLK BOUNDARY (SLOW->FAST)
  //SYNC FLOPS FOR SYS_CLK_ACT
  always @ (posedge clk_in or posedge reset_in) begin
	if(reset_in==1) begin
		sys_clk_act_sync_1 <= 0;
		sys_clk_act_sync_2 <= 0;
	end
	else begin	
		sys_clk_act_sync_1 <= sys_clk_activated;
		sys_clk_act_sync_2 <= sys_clk_act_sync_1;
	end
  end  
  //2 SYNC FLOPS FOR CLK_SWITCH_REQUEST (CLK->SLOW)
  always @ (posedge tck or posedge reset_in) begin
	if(reset_in==1) begin
		clk_switch_request_sync_1_tck <= 0;
		clk_switch_request_sync_2_tck <= 0;
	end
	else begin
		clk_switch_request_sync_1_tck <= clk_switch_request;
		clk_switch_request_sync_2_tck <= clk_switch_request_sync_1_tck;
	end	
  end
  //2 SYNC FLOPS FOR CLK_SWITCH_REQUEST (CLK->FAST)
  always @ (posedge clk_in or posedge reset_in) begin
	if(reset_in==1) begin
		clk_switch_request_sync_1_sys <= 0;
		clk_switch_request_sync_2_sys <= 0;
	end
	else begin
		clk_switch_request_sync_1_sys <= clk_switch_request;
		clk_switch_request_sync_2_sys <= clk_switch_request_sync_1_sys;
	end	
  end


  always @ (posedge clk_in or posedge reset_in) begin 
	if (reset_in==1) begin
		all_stalled_sys_rise <= 1;
	end
	else if (sys_clk_act_sync_2 != clk_switch_request_sync_2_sys) begin
		all_stalled_sys_rise <= 1;
	end
	else if ((sys_clk_act_sync_2 == 1) && (clk_switch_request_sync_2_sys == 1)) begin
		all_stalled_sys_rise <= 0;
	end
  end

  //FALLING EDGE SYS CLK. In case we need to delay the clock gating signal by an extra half cycle
  always @ (negedge clk_in or posedge reset_in) begin
	if (reset_in==1)
		all_stalled_sys_fall <= 0;
	else
		all_stalled_sys_fall <= all_stalled_sys_rise;
  end

  always @ (posedge clk or posedge reset_in) begin
    	if (reset_in==1) begin
		cgra_stalled <= 0;
		stall_restore <= 0;
       		clk_switch_request <= 0;
		rd_delay_reg <= 2;
		TST <= 0;
		counter <= 0;
		config_addr_out <= 0;
		config_data_out <= 0;
		config_data_jtag_in <= 0;
		delay_sel <= 2'b0;
		read <= 0;
		write <= 0;
		analog_wr_en <= 0;
		analog_data_in <= 0;
		analog_addr <= 0;
		state <= ready;
    	end

    	else begin
    		if (state==ready) begin
    			case(op)
			NOP: begin
				config_addr_out <= config_addr_out;
				config_data_out <= config_data_out;
				config_data_jtag_in <= config_data_jtag_in;	
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			write_config: begin
				config_addr_out <= config_addr_jtag_out;
				config_data_out <= config_data_jtag_out; 
				read <= 0;
				write <= 1;
				analog_wr_en <= 0;
			end
			read_config: begin
				config_addr_out <= config_addr_jtag_out;
				config_data_out <= config_data_jtag_out; 
				counter <= rd_delay_reg-1;
				state <= reading;
				read <= 1;
				write <= 0;
				analog_wr_en <= 0;
			end
			write_A050: begin
				config_data_jtag_in <= 32'hA050;
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			write_TST: begin
				TST <= config_data_jtag_out;
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			read_TST: begin
				config_data_jtag_in <= TST;
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			global_reset: begin
				state <= resetting;
				counter <= (config_data_jtag_out > 0) ? config_data_jtag_out-1 : 32'd19;
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			read_stall: begin
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
				config_data_jtag_in <= {28'b0, cgra_stalled};
			end
			write_stall: begin
				cgra_stalled <= config_data_jtag_out[3:0];
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end			
			advance_clk: begin
				if (config_data_jtag_out > 0) begin
					if (|(cgra_stalled & config_addr_jtag_out[3:0])) begin
						counter <= config_data_jtag_out-1;
						state <= advancing_clk;	
						stall_restore <= cgra_stalled;
						cgra_stalled[0] <= config_addr_jtag_out[0] ? 1'b0 : cgra_stalled[0];
						cgra_stalled[1] <= config_addr_jtag_out[1] ? 1'b0 : cgra_stalled[1];
						cgra_stalled[2] <= config_addr_jtag_out[2] ? 1'b0 : cgra_stalled[2];
						cgra_stalled[3] <= config_addr_jtag_out[3] ? 1'b0 : cgra_stalled[3];
					end
				end
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			read_clk_domain: begin
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
				config_data_jtag_in <= {31'b0, sys_clk_activated};
			end
			switch_clk: begin
				if(config_data_jtag_out[0] != clk_switch_request) begin
					clk_switch_request <= config_data_jtag_out[0];
					state <= switching_clk;
					counter <= 32'd20; 
				end
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			wr_rd_delay_reg: begin
				//Prevent underflow by setting this to 1 if the input data is 0.
				rd_delay_reg <= (config_data_jtag_out > 0) ? config_data_jtag_out : 1;
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			rd_rd_delay_reg: begin
				config_data_jtag_in <= rd_delay_reg;
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			wr_delay_sel_reg: begin
				delay_sel <= config_data_jtag_out[1:0];
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			rd_delay_sel_reg: begin
				config_data_jtag_in <= {30'b0,delay_sel};
				read <= 0;
				write <= 0;
				analog_wr_en <= 0;
			end
			wr_analog_reg: begin
				analog_wr_en <= 1;
				analog_data_in <= config_data_jtag_out;
				analog_addr <= config_addr_jtag_out;
				read <= 0;
				write <= 0;	
			end
			rd_analog_reg: begin	
				case(config_addr_jtag_out)
					'd0: config_data_jtag_in <= analog_r0;
					'd1: config_data_jtag_in <= analog_r1;
					'd2: config_data_jtag_in <= analog_r2;
					'd3: config_data_jtag_in <= analog_r3;
					'd4: config_data_jtag_in <= analog_r4;
					'd5: config_data_jtag_in <= analog_r5;
					'd6: config_data_jtag_in <= analog_r6;
					'd7: config_data_jtag_in <= analog_r7;
					'd8: config_data_jtag_in <= analog_r8;
					'd9: config_data_jtag_in <= analog_r9;
					'd10: config_data_jtag_in <= analog_r10;
					'd11: config_data_jtag_in <= analog_r11;
					'd12: config_data_jtag_in <= analog_r12;
					'd13: config_data_jtag_in <= analog_r13;
					'd14: config_data_jtag_in <= analog_r14;
					default: config_data_jtag_in <= 0;
				endcase
				analog_wr_en <= 0;
				read <= 0;
				write <= 0;
			end
			default: begin
				config_addr_out <= config_addr_out;
				config_data_out <= config_data_out;
				config_data_jtag_in <= config_data_jtag_in;	
				read <= 0;
				write <= 0;
			end
			endcase
		end
		//Counter for any state that needs to block incoming instructions
		else begin
			config_addr_out <= config_addr_out;
			if (counter > 0) begin
				counter <= counter-1;
			end
			else begin
				state <= ready;
				if (state == reading) begin 
					config_data_jtag_in <= config_data_in;
					read <= 0;
				end
				else if (state == advancing_clk)
					cgra_stalled <= stall_restore;
			end
		end    	
  	end
  end
	

endmodule
