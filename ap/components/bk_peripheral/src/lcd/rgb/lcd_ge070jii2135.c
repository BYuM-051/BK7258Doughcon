// Copyright 2020-2021 Beken
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <driver/gpio.h>
#include <components/media_types.h>
#include <driver/lcd_types.h>
#include <driver/lcd.h>
#include "lcd_panel_devices.h"
#include "gpio_map.h"
#include "gpio_driver.h"

// GE070JII2135-A2: 7.0" 1024x600 RGB panel
// Datasheet timing (section 3.3.3):
//   DCLK typ=51.2MHz, HS_Blanking=160, HFP=160, VS_Blanking=23, VFP=12
//   Data latched at falling edge of DCLK (Note 3)
// th = 1024 + 160(HBP+HPW) + 160(HFP) = 1344 (typ)
// tv = 600 + 23(VBP+VPW) + 12(VFP)   = 635  (typ)

static const lcd_rgb_t lcd_rgb =
{
	.clk = LCD_54M,
	.data_out_clk_edge = POSEDGE_OUTPUT,

	.hsync_pulse_width = 2,
	.vsync_pulse_width = 2,
	.hsync_back_porch  = 158,
	.hsync_front_porch = 160,
	.vsync_back_porch  = 21,
	.vsync_front_porch = 12,
};

const lcd_device_t lcd_device_ge070jii2135 =
{
	.id      = LCD_DEVICE_GE070JII2135,
	.name    = "ge070jii2135",
	.type    = LCD_TYPE_RGB565,
	.width   = 1024,
	.height  = 600,
	.rgb     = &lcd_rgb,
	.out_fmt = PIXEL_FMT_RGB565,
	.init    = NULL,
	.off     = NULL,
};
