/*
 * Copyright 2012 Nouveau Community
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Martin Peres <martin.peres@labri.fr>
 *          Ben Skeggs
 */

#include <subdev/bus.h>

struct nvc0_bus_priv {
	struct nouveau_bus base;
};

static void
nvc0_bus_intr(struct nouveau_subdev *subdev)
{
	struct nouveau_bus *pbus = nouveau_bus(subdev);
	u32 stat = nv_rd32(pbus, 0x001100) & nv_rd32(pbus, 0x001140);

	if (stat & 0x0000000e) {
		u32 addr = nv_rd32(pbus, 0x009084);
		u32 data = nv_rd32(pbus, 0x009088);

		nv_error(pbus, "MMIO %s of 0x%08x FAULT at 0x%06x [ %s%s%s]\n",
			 (addr & 0x00000002) ? "write" : "read", data,
			 (addr & 0x00fffffc),
			 (stat & 0x00000002) ? "!ENGINE " : "",
			 (stat & 0x00000004) ? "IBUS " : "",
			 (stat & 0x00000008) ? "TIMEOUT " : "");

		nv_wr32(pbus, 0x009084, 0x00000000);
		nv_wr32(pbus, 0x001100, (stat & 0x0000000e));
		stat &= ~0x0000000e;
	}

	if (stat) {
		nv_error(pbus, "unknown intr 0x%08x\n", stat);
		nv_mask(pbus, 0x001140, stat, 0x00000000);
	}
}

static int
nvc0_bus_init(struct nouveau_object *object)
{
	struct nvc0_bus_priv *priv = (void *)object;
	int ret;

	ret = nouveau_bus_init(&priv->base);
	if (ret)
		return ret;

	nv_wr32(priv, 0x001100, 0xffffffff);
	nv_wr32(priv, 0x001140, 0x0000000e);
	return 0;
}

static int
nvc0_bus_ctor(struct nouveau_object *parent, struct nouveau_object *engine,
	      struct nouveau_oclass *oclass, void *data, u32 size,
	      struct nouveau_object **pobject)
{
	struct nvc0_bus_priv *priv;
	int ret;

	ret = nouveau_bus_create(parent, engine, oclass, &priv);
	*pobject = nv_object(priv);
	if (ret)
		return ret;

	nv_subdev(priv)->intr = nvc0_bus_intr;
	return 0;
}

struct nouveau_oclass
nvc0_bus_oclass = {
	.handle = NV_SUBDEV(BUS, 0xc0),
	.ofuncs = &(struct nouveau_ofuncs) {
		.ctor = nvc0_bus_ctor,
		.dtor = _nouveau_bus_dtor,
		.init = nvc0_bus_init,
		.fini = _nouveau_bus_fini,
	},
};
