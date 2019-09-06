// SPDX-License-Identifier: GPL-2.0
/*
 * (C) COPYRIGHT 2018 ARM Limited. All rights reserved.
 * Author: James.Qian.Wang <james.qian.wang@arm.com>
 *
 */
#include <drm/drm_print.h>

#include "komeda_dev.h"
#include "komeda_pipeline.h"

/** komeda_pipeline_add - Add a pipeline to &komeda_dev */
struct komeda_pipeline *
komeda_pipeline_add(struct komeda_dev *mdev, size_t size,
		    struct komeda_pipeline_funcs *funcs)
{
	struct komeda_pipeline *pipe;

	if (mdev->n_pipelines + 1 > KOMEDA_MAX_PIPELINES) {
		DRM_ERROR("Exceed max support %d pipelines.\n",
			  KOMEDA_MAX_PIPELINES);
		return NULL;
	}

	if (size < sizeof(*pipe)) {
		DRM_ERROR("Request pipeline size too small.\n");
		return NULL;
	}

	pipe = devm_kzalloc(mdev->dev, size, GFP_KERNEL);
	if (!pipe)
		return NULL;

	pipe->mdev = mdev;
	pipe->id   = mdev->n_pipelines;
	pipe->funcs = funcs;

	mdev->pipelines[mdev->n_pipelines] = pipe;
	mdev->n_pipelines++;

	return pipe;
}

void komeda_pipeline_destroy(struct komeda_dev *mdev,
			     struct komeda_pipeline *pipe)
{
	struct komeda_component *c;
	int i;

	dp_for_each_set_bit(i, pipe->avail_comps) {
		c = komeda_pipeline_get_component(pipe, i);
		komeda_component_destroy(mdev, c);
	}

	clk_put(pipe->pxlclk);
	clk_put(pipe->aclk);

	of_node_put(pipe->of_output_dev);
	of_node_put(pipe->of_output_port);
	of_node_put(pipe->of_node);

	devm_kfree(mdev->dev, pipe);
}

struct komeda_component **
komeda_pipeline_get_component_pos(struct komeda_pipeline *pipe, int id)
{
	struct komeda_dev *mdev = pipe->mdev;
	struct komeda_pipeline *temp = NULL;
	struct komeda_component **pos = NULL;

	switch (id) {
	case KOMEDA_COMPONENT_LAYER0:
	case KOMEDA_COMPONENT_LAYER1:
	case KOMEDA_COMPONENT_LAYER2:
	case KOMEDA_COMPONENT_LAYER3:
		pos = to_cpos(pipe->layers[id - KOMEDA_COMPONENT_LAYER0]);
		break;
	case KOMEDA_COMPONENT_WB_LAYER:
		pos = to_cpos(pipe->wb_layer);
		break;
	case KOMEDA_COMPONENT_COMPIZ0:
	case KOMEDA_COMPONENT_COMPIZ1:
		temp = mdev->pipelines[id - KOMEDA_COMPONENT_COMPIZ0];
		if (!temp) {
			DRM_ERROR("compiz-%d doesn't exist.\n", id);
			return NULL;
		}
		pos = to_cpos(temp->compiz);
		break;
	case KOMEDA_COMPONENT_SCALER0:
	case KOMEDA_COMPONENT_SCALER1:
		pos = to_cpos(pipe->scalers[id - KOMEDA_COMPONENT_SCALER0]);
		break;
	case KOMEDA_COMPONENT_IPS0:
	case KOMEDA_COMPONENT_IPS1:
		temp = mdev->pipelines[id - KOMEDA_COMPONENT_IPS0];
		if (!temp) {
			DRM_ERROR("ips-%d doesn't exist.\n", id);
			return NULL;
		}
		pos = to_cpos(temp->improc);
		break;
	case KOMEDA_COMPONENT_TIMING_CTRLR:
		pos = to_cpos(pipe->ctrlr);
		break;
	default:
		pos = NULL;
		DRM_ERROR("Unknown pipeline resource ID: %d.\n", id);
		break;
	}

	return pos;
}

struct komeda_component *
komeda_pipeline_get_component(struct komeda_pipeline *pipe, int id)
{
	struct komeda_component **pos = NULL;
	struct komeda_component *c = NULL;

	pos = komeda_pipeline_get_component_pos(pipe, id);
	if (pos)
		c = *pos;

	return c;
}

/** komeda_component_add - Add a component to &komeda_pipeline */
struct komeda_component *
komeda_component_add(struct komeda_pipeline *pipe,
		     size_t comp_sz, u32 id, u32 hw_id,
		     struct komeda_component_funcs *funcs,
		     u8 max_active_inputs, u32 supported_inputs,
		     u8 max_active_outputs, u32 __iomem *reg,
		     const char *name_fmt, ...)
{
	struct komeda_component **pos;
	struct komeda_component *c;
	int idx, *num = NULL;

	if (max_active_inputs > KOMEDA_COMPONENT_N_INPUTS) {
		WARN(1, "please large KOMEDA_COMPONENT_N_INPUTS to %d.\n",
		     max_active_inputs);
		return NULL;
	}

	pos = komeda_pipeline_get_component_pos(pipe, id);
	if (!pos || (*pos))
		return NULL;

	if (has_bit(id, KOMEDA_PIPELINE_LAYERS)) {
		idx = id - KOMEDA_COMPONENT_LAYER0;
		num = &pipe->n_layers;
		if (idx != pipe->n_layers) {
			DRM_ERROR("please add Layer by id sequence.\n");
			return NULL;
		}
	} else if (has_bit(id,  KOMEDA_PIPELINE_SCALERS)) {
		idx = id - KOMEDA_COMPONENT_SCALER0;
		num = &pipe->n_scalers;
		if (idx != pipe->n_scalers) {
			DRM_ERROR("please add Scaler by id sequence.\n");
			return NULL;
		}
	}

	c = devm_kzalloc(pipe->mdev->dev, comp_sz, GFP_KERNEL);
	if (!c)
		return NULL;

	c->id = id;
	c->hw_id = hw_id;
	c->reg = reg;
	c->pipeline = pipe;
	c->max_active_inputs = max_active_inputs;
	c->max_active_outputs = max_active_outputs;
	c->supported_inputs = supported_inputs;
	c->funcs = funcs;

	if (name_fmt) {
		va_list args;

		va_start(args, name_fmt);
		vsnprintf(c->name, sizeof(c->name), name_fmt, args);
		va_end(args);
	}

	if (num)
		*num = *num + 1;

	pipe->avail_comps |= BIT(c->id);
	*pos = c;

	return c;
}

void komeda_component_destroy(struct komeda_dev *mdev,
			      struct komeda_component *c)
{
	devm_kfree(mdev->dev, c);
}
