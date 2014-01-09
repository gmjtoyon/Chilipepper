-- -------------------------------------------------------------
-- 
-- File Name: C:\GIT\QPSK_Pcore\Labs\Lab_1\MATLAB\codegen\dac_driver\hdlsrc\dac_driver_fixpt.vhd
-- Created: 2013-12-11 15:36:44
-- 
-- Generated by MATLAB 8.3, MATLAB Coder 2.6 and HDL Coder 3.4
-- 
-- 
-- 
-- -------------------------------------------------------------
-- Rate and Clocking Details
-- -------------------------------------------------------------
-- Design base rate: 1
-- 
-- 
-- Clock Enable  Sample Time
-- -------------------------------------------------------------
-- ce_out        1
-- -------------------------------------------------------------
-- 
-- 
-- Output Signal                 Clock Enable  Sample Time
-- -------------------------------------------------------------
-- tx_iq_sel                     ce_out        1
-- txd                           ce_out        1
-- blinky                        ce_out        1
-- -------------------------------------------------------------
-- 
-- -------------------------------------------------------------


-- -------------------------------------------------------------
-- 
-- Module: dac_driver_fixpt
-- Source Path: dac_driver_fixpt
-- Hierarchy Level: 0
-- 
-- -------------------------------------------------------------
LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;
USE IEEE.numeric_std.ALL;

ENTITY dac_driver_fixpt IS
  PORT( clk                               :   IN    std_logic;
        reset                             :   IN    std_logic;
        clk_enable                        :   IN    std_logic;
        tx_i                              :   IN    std_logic_vector(11 DOWNTO 0);  -- sfix12
        tx_q                              :   IN    std_logic_vector(11 DOWNTO 0);  -- sfix12
        dac_en                            :   IN    std_logic;  -- ufix1
        ce_out                            :   OUT   std_logic;
        tx_iq_sel                         :   OUT   std_logic;  -- ufix1
        txd                               :   OUT   std_logic_vector(11 DOWNTO 0);  -- sfix12
        blinky                            :   OUT   std_logic  -- ufix1
        );
END dac_driver_fixpt;


ARCHITECTURE rtl OF dac_driver_fixpt IS

  -- Functions
  -- HDLCODER_TO_STDLOGIC 
  FUNCTION hdlcoder_to_stdlogic(arg: boolean) RETURN std_logic IS
  BEGIN
    IF arg THEN
      RETURN '1';
    ELSE
      RETURN '0';
    END IF;
  END FUNCTION;


  -- Signals
  SIGNAL enb                              : std_logic;
  SIGNAL tmp                              : std_logic;
  SIGNAL count                            : std_logic;  -- ufix1
  SIGNAL tmp_1                            : std_logic;  -- ufix1
  SIGNAL count_1                          : std_logic;  -- ufix1
  SIGNAL tmp_2                            : std_logic;  -- ufix1
  SIGNAL tmp_3                            : std_logic;  -- ufix1
  SIGNAL tx_iq_sel_1                      : std_logic;  -- ufix1
  SIGNAL tmp_4                            : std_logic;
  SIGNAL tx_q_signed                      : signed(11 DOWNTO 0);  -- sfix12
  SIGNAL tx_i_signed                      : signed(11 DOWNTO 0);  -- sfix12
  SIGNAL tmp_5                            : signed(11 DOWNTO 0);  -- sfix12
  SIGNAL tmp_6                            : signed(11 DOWNTO 0);  -- sfix12
  SIGNAL p9_tmp                           : signed(11 DOWNTO 0);  -- sfix12
  SIGNAL tmp_7                            : unsigned(24 DOWNTO 0);  -- ufix25
  SIGNAL blinky_cnt                       : unsigned(24 DOWNTO 0);  -- ufix25
  SIGNAL blinky_cnt_1                     : unsigned(24 DOWNTO 0);  -- ufix25
  SIGNAL tmp_8                            : std_logic;
  SIGNAL blinky_1                         : std_logic;  -- ufix1
  SIGNAL p16blinky_mul_temp               : unsigned(38 DOWNTO 0);  -- ufix39_En37
  SIGNAL p16blinky_cast                   : unsigned(1 DOWNTO 0);  -- ufix2
  SIGNAL blinky_2                         : std_logic;  -- ufix1

BEGIN
  
  tmp <= '1' WHEN dac_en = '1' ELSE
      '0';

  enb <= clk_enable;

  tmp_1 <= hdlcoder_to_stdlogic( NOT (count /= '0'));

  count_1 <= tmp_1;

  count_reg_process : PROCESS (clk, reset)
  BEGIN
    IF reset = '1' THEN
      count <= '0';
    ELSIF clk'EVENT AND clk = '1' THEN
      IF enb = '1' THEN
        count <= count_1;
      END IF;
    END IF;
  END PROCESS count_reg_process;


  tmp_2 <= count;

  
  tmp_3 <= tmp_2 WHEN tmp = '1' ELSE
      '0';

  tx_iq_sel_1 <= tmp_3;

  tx_iq_sel <= tx_iq_sel_1;

  --%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  --                                                                          %
  --       Generated by MATLAB 8.3, MATLAB Coder 2.6 and HDL Coder 3.4        %
  --                                                                          %
  --%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  --interleave I and Q channels into txd
  --DAC runs at 2x the input rate
  
  tmp_4 <= '1' WHEN dac_en = '1' ELSE
      '0';

  tx_q_signed <= signed(tx_q);

  tx_i_signed <= signed(tx_i);

  --HDL code generation from MATLAB function: dac_driver_fixpt
  tmp_5 <= tx_i_signed;

  
  p9_tmp <= tmp_5 WHEN tmp_2 = '0' ELSE
      tx_q_signed;
  
  tmp_6 <= p9_tmp WHEN tmp_4 = '1' ELSE
      to_signed(2#000000000000#, 12);

  txd <= std_logic_vector(tmp_6);

  blinky_cnt_reg_process : PROCESS (clk, reset)
  BEGIN
    IF reset = '1' THEN
      blinky_cnt <= to_unsigned(2#0000000000000000000000000#, 25);
    ELSIF clk'EVENT AND clk = '1' THEN
      IF enb = '1' THEN
        blinky_cnt <= tmp_7;
      END IF;
    END IF;
  END PROCESS blinky_cnt_reg_process;


  blinky_cnt_1 <= blinky_cnt + 1;

  
  tmp_8 <= '1' WHEN blinky_cnt_1 = 20000000 ELSE
      '0';

  
  tmp_7 <= to_unsigned(2#0000000000000000000000000#, 25) WHEN tmp_8 = '1' ELSE
      blinky_cnt_1;

  p16blinky_mul_temp <= tmp_7 * to_unsigned(2#11010110101111#, 14);
  p16blinky_cast <= p16blinky_mul_temp(38 DOWNTO 37);
  blinky_1 <= p16blinky_cast(0);

  blinky_2 <= blinky_1;

  blinky <= blinky_2;

  ce_out <= clk_enable;

END rtl;

