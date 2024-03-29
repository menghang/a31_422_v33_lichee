#include "disp_hdmi.h"
#include "disp_display.h"
#include "disp_event.h"
#include "disp_de.h"
#include "disp_tv.h"
#include "disp_lcd.h"
#include "disp_clk.h"


__s32 Display_Hdmi_Init(void)
{
    __s32 ret;
    __u32 value;
    
    ret = OSAL_Script_FetchParser_Data("hdmi_para", "hdmi_used", (int*)&value, 1);
    if(ret < 0)
    {
        DE_WRN("fetch script data hdmi_para.hdmi_used fail\n");
    }
    else
    {
        DE_INF("hdmi_para.hdmi_used = %d\n", value);
        gdisp.screen[0].hdmi_used = value;
        gdisp.screen[1].hdmi_used = value;

        if(gdisp.screen[0].hdmi_used || gdisp.screen[1].hdmi_used)
        {
            hdmi_clk_init();
        }
        gdisp.screen[0].hdmi_mode = DISP_TV_MOD_720P_50HZ;
        gdisp.screen[1].hdmi_mode = DISP_TV_MOD_720P_50HZ;
    }

    return DIS_SUCCESS;
}

__s32 Display_Hdmi_Exit(void)
{
    if(gdisp.screen[0].hdmi_used || gdisp.screen[1].hdmi_used)
    {
        hdmi_clk_exit();
    }
    
	return DIS_SUCCESS;
}

__s32 BSP_disp_hdmi_open(__u32 sel)
{
    if((gdisp.screen[sel].hdmi_used) && (!(gdisp.screen[sel].status & HDMI_ON)))
    {
    	__disp_tv_mode_t     tv_mod;
        
    	tv_mod = gdisp.screen[sel].hdmi_mode;

        hdmi_clk_on();
    	lcdc_clk_on(sel);
    	image_clk_on(sel);
		Image_open(sel);//set image normal channel start bit , because every de_clk_off( )will reset this bit
    	disp_clk_cfg(sel,DISP_OUTPUT_TYPE_HDMI, tv_mod);

        BSP_disp_set_output_csc(sel, DISP_OUTPUT_TYPE_HDMI);
    	DE_BE_set_display_size(sel, tv_mode_to_width(tv_mod), tv_mode_to_height(tv_mod));
    	DE_BE_Output_Select(sel, sel);

        if(BSP_disp_cmu_get_enable(sel) ==1)
        {
            IEP_CMU_Set_Imgsize(sel, BSP_disp_get_screen_width(sel), BSP_disp_get_screen_height(sel));
        }
        
    	tcon1_set_hdmi_mode(sel,tv_mod);		 	 
    	tcon1_open(sel);
        if(gdisp.init_para.hdmi_open)
    	{
            gdisp.init_para.hdmi_open();
    	}
    	else
    	{
    	    DE_WRN("Hdmi_open is NULL\n");
    	    return -1;
    	}
    	
    	Disp_Switch_Dram_Mode(DISP_OUTPUT_TYPE_HDMI, tv_mod);

    	gdisp.screen[sel].b_out_interlace = Disp_get_screen_scan_mode(tv_mod);
    	gdisp.screen[sel].status |= HDMI_ON;
        gdisp.screen[sel].lcdc_status |= LCDC_TCON1_USED;
        gdisp.screen[sel].output_type = DISP_OUTPUT_TYPE_HDMI;

        Disp_set_out_interlace(sel);
#ifdef __LINUX_OSAL__
        Display_set_fb_timming(sel);
#endif
    return DIS_SUCCESS;
    }
    
    return DIS_NOT_SUPPORT;
}

__s32 BSP_disp_hdmi_close(__u32 sel)
{
    if((gdisp.screen[sel].hdmi_used) && (gdisp.screen[sel].status & HDMI_ON))
    {
        if(gdisp.init_para.hdmi_close)
    	{
            gdisp.init_para.hdmi_close();
    	}
    	else
    	{
    	    DE_WRN("Hdmi_close is NULL\n");
    	    return -1;
    	}
        Image_close(sel);
    	tcon1_close(sel);

    	image_clk_off(sel);
    	lcdc_clk_off(sel);
    	hdmi_clk_off();

        gdisp.screen[sel].b_out_interlace = 0;
        gdisp.screen[sel].lcdc_status &= LCDC_TCON1_USED_MASK;
        gdisp.screen[sel].status &= HDMI_OFF;
        gdisp.screen[sel].output_type = DISP_OUTPUT_TYPE_NONE;
        gdisp.screen[sel].pll_use_status &= ((gdisp.screen[sel].pll_use_status == VIDEO_PLL0_USED)? VIDEO_PLL0_USED_MASK : VIDEO_PLL1_USED_MASK);

        Disp_set_out_interlace(sel);

        return DIS_SUCCESS;
    }

    return DIS_NOT_SUPPORT;
}

__s32 BSP_disp_hdmi_set_mode(__u32 sel, __disp_tv_mode_t  mode)
{ 	
    if(mode >= DISP_TV_MODE_NUM)
    {
        DE_WRN("unsupported hdmi mode:%d in BSP_disp_hdmi_set_mode\n", mode);
        return DIS_FAIL;
    }

    if(gdisp.screen[sel].hdmi_used)
    {
        if(gdisp.init_para.hdmi_set_mode)
        {
          gdisp.init_para.hdmi_set_mode(mode);
        }
        else
        {
          DE_WRN("hdmi_set_mode is NULL\n");
          return -1;
        }

        gdisp.screen[sel].hdmi_mode = mode;
        gdisp.screen[sel].output_type = DISP_OUTPUT_TYPE_HDMI;

        return DIS_SUCCESS;
    }

	return DIS_NOT_SUPPORT;
}

__s32 BSP_disp_hdmi_get_mode(__u32 sel)
{   
    return gdisp.screen[sel].hdmi_mode;
}

__s32 BSP_disp_hdmi_check_support_mode(__u32 sel, __u8  mode)
{ 
	if((gdisp.screen[sel].hdmi_used) && (gdisp.init_para.hdmi_mode_support))
	{
	    return gdisp.init_para.hdmi_mode_support(mode);
	}
	else
	{
	    DE_WRN("hdmi_mode_support is NULL\n");
	    return DIS_NOT_SUPPORT;
	}
}

__s32 BSP_disp_hdmi_get_hpd_status(__u32 sel)
{
	if((gdisp.screen[sel].hdmi_used) && (gdisp.init_para.hdmi_get_HPD_status))
	{
	    return gdisp.init_para.hdmi_get_HPD_status();
	}
	else
	{
	    DE_WRN("hdmi_get_HPD_status is NULL\n");
	    return -1;
	}
}

__s32 BSP_disp_hdmi_set_src(__u32 sel, __disp_lcdc_src_t src)
{
    if(gdisp.screen[sel].hdmi_used)
    {
        switch (src)
        {
            case DISP_LCDC_SRC_DE_CH1:
                tcon1_src_select(sel, LCD_SRC_BE0);
                break;

            case DISP_LCDC_SRC_DE_CH2:
                tcon1_src_select(sel, LCD_SRC_BE1);
                break;

            case DISP_LCDC_SRC_BLUE:
                tcon1_src_select(sel, LCD_SRC_BLUE);
                break;

            default:
                DE_WRN("not supported lcdc src:%d in BSP_disp_tv_set_src\n", src);
                return DIS_NOT_SUPPORT;
        }
        return DIS_SUCCESS;
    }

    return DIS_NOT_SUPPORT;
}

__s32 BSP_disp_hdmi_suspend()
{
    if(gdisp.init_para.hdmi_suspend)
    {
        return gdisp.init_para.hdmi_suspend();
    }

    return -1;
}

__s32 BSP_disp_hdmi_resume()
{
    if(gdisp.init_para.hdmi_resume)
    {
        return gdisp.init_para.hdmi_resume();
    }

    return -1;
}

__s32 BSP_disp_set_hdmi_func(__disp_hdmi_func * func)
{
    gdisp.init_para.hdmi_open = func->hdmi_open;
    gdisp.init_para.hdmi_close = func->hdmi_close;
    gdisp.init_para.hdmi_set_mode = func->hdmi_set_mode;
    gdisp.init_para.hdmi_mode_support = func->hdmi_mode_support;
    gdisp.init_para.hdmi_get_HPD_status = func->hdmi_get_HPD_status;
    gdisp.init_para.hdmi_set_pll = func->hdmi_set_pll;
    gdisp.init_para.hdmi_suspend= func->hdmi_suspend;
    gdisp.init_para.hdmi_resume= func->hdmi_resume;
    
    return DIS_SUCCESS;
}

