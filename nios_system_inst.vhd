  --Example instantiation for system 'nios_system'
  nios_system_inst : nios_system
    port map(
      LCD_E_from_the_LCD => LCD_E_from_the_LCD,
      LCD_RS_from_the_LCD => LCD_RS_from_the_LCD,
      LCD_RW_from_the_LCD => LCD_RW_from_the_LCD,
      LCD_data_to_and_from_the_LCD => LCD_data_to_and_from_the_LCD,
      zs_addr_from_the_sdram => zs_addr_from_the_sdram,
      zs_ba_from_the_sdram => zs_ba_from_the_sdram,
      zs_cas_n_from_the_sdram => zs_cas_n_from_the_sdram,
      zs_cke_from_the_sdram => zs_cke_from_the_sdram,
      zs_cs_n_from_the_sdram => zs_cs_n_from_the_sdram,
      zs_dq_to_and_from_the_sdram => zs_dq_to_and_from_the_sdram,
      zs_dqm_from_the_sdram => zs_dqm_from_the_sdram,
      zs_ras_n_from_the_sdram => zs_ras_n_from_the_sdram,
      zs_we_n_from_the_sdram => zs_we_n_from_the_sdram,
      clk => clk,
      reset_n => reset_n
    );


