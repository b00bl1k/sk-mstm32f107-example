/*
 * This file is part of the sk-mstm32f107-example project.
 *
 * Copyright (C) 2016 Alexey Ryabov <6006l1k@gmail.com>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KSZ8721IF_H__
#define __KSZ8721IF_H__

#include "lwip/def.h"

err_t ksz8721if_init(struct netif *);
void ksz8721if_poll(int phy, struct netif *);

#endif /* __KSZ8721IF_H__ */
