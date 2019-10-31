#include "stabilization_smart_layer.h"
#include "utilities.h"
#include "logging.h"
#include <iostream>


stabilization_smart_layer::stabilization_smart_layer()
{
	// Initialize travel args
	smart_layer_args_ = smart_layer_args();
	// initialize layer/height tracking variables
	is_layer_change_wait_ = false;
	current_layer_ = 0;
	current_height_increment_ = 0;
	has_one_extrusion_speed_ = true;
	last_tested_gcode_number_ = -1;
	stabilization_x_ = 0;
	stabilization_y_ = 0;
	current_layer_saved_extrusion_speed_ = -1;
	standard_layer_trigger_distance_ = 0.0;
	fastest_extrusion_speed_ = -1;
	slowest_extrusion_speed_ = -1;
	last_snapshot_layer_ = 0;
	last_snapshot_height_increment_ = 0;
	trigger_position_args default_args;
	closest_positions_.initialize(default_args);
}

stabilization_smart_layer::stabilization_smart_layer(
	gcode_position_args position_args, stabilization_args stab_args, smart_layer_args mt_args, progressCallback progress
) :stabilization(position_args, stab_args, progress)
{
	is_layer_change_wait_ = false;
	current_layer_ = 0;
	current_height_increment_ = 0;
	last_snapshot_layer_ = 0;
	last_snapshot_height_increment_ = 0;
	has_one_extrusion_speed_ = true;
	last_tested_gcode_number_ = -1;
	fastest_extrusion_speed_ = -1;
	slowest_extrusion_speed_ = -1;

	smart_layer_args_ = mt_args;
	// initialize closest extrusion/travel tracking structs
	stabilization_x_ = 0;
	stabilization_y_ = 0;
	current_layer_saved_extrusion_speed_ = -1;
	standard_layer_trigger_distance_ = 0.0;

	trigger_position_args default_args;
	default_args.type = mt_args.smart_layer_trigger_type;
	default_args.minimum_speed = mt_args.speed_threshold;
	default_args.snap_to_print_high_quality = mt_args.snap_to_print_high_quality;
	default_args.x_stabilization_disabled = stab_args.x_stabilization_disabled;
	default_args.y_stabilization_disabled = stab_args.y_stabilization_disabled;
	closest_positions_.initialize(default_args);
	last_snapshot_initial_position_.is_empty = true;
	update_stabilization_coordinates();
}

stabilization_smart_layer::stabilization_smart_layer(
	gcode_position_args position_args, stabilization_args stab_args, smart_layer_args mt_args, pythonGetCoordinatesCallback get_coordinates, PyObject* py_get_coordinates_callback, pythonProgressCallback progress, PyObject* py_progress_callback
) : stabilization(position_args, stab_args, get_coordinates, py_get_coordinates_callback, progress, py_progress_callback)
{
	is_layer_change_wait_ = false;
	current_layer_ = 0;
	current_height_increment_ = 0;
	last_snapshot_layer_ = 0;
	last_snapshot_height_increment_ = 0;
	has_one_extrusion_speed_ = true;
	last_tested_gcode_number_ = -1;
	fastest_extrusion_speed_ = -1;
	slowest_extrusion_speed_ = -1;

	smart_layer_args_ = mt_args;
	// Get the initial stabilization coordinates
	stabilization_x_ = 0;
	stabilization_y_ = 0;
	current_layer_saved_extrusion_speed_ = -1;
	standard_layer_trigger_distance_ = 0.0;

	trigger_position_args default_args;
	default_args.type = mt_args.smart_layer_trigger_type;
	default_args.minimum_speed = mt_args.speed_threshold;
	default_args.snap_to_print_high_quality = mt_args.snap_to_print_high_quality;
	default_args.x_stabilization_disabled = stab_args.x_stabilization_disabled;
	default_args.y_stabilization_disabled = stab_args.y_stabilization_disabled;
	closest_positions_.initialize(default_args);
	last_snapshot_initial_position_.is_empty = true;
	update_stabilization_coordinates();
}

stabilization_smart_layer::stabilization_smart_layer(const stabilization_smart_layer &source)
{
	
}

stabilization_smart_layer::~stabilization_smart_layer()
{
	
}

void stabilization_smart_layer::update_stabilization_coordinates()
{
	const bool snap_to_print_smooth = smart_layer_args_.smart_layer_trigger_type == trigger_type_snap_to_print && smart_layer_args_.snap_to_print_smooth;
	const bool stabilization_disabled = stabilization_args_.x_stabilization_disabled && stabilization_args_.y_stabilization_disabled;
	if (
		(stabilization_disabled || snap_to_print_smooth)
		&& !last_snapshot_initial_position_.is_empty
	)
	{
		stabilization_x_ = last_snapshot_initial_position_.x;
		stabilization_y_ = last_snapshot_initial_position_.y;
	}
	else
	{
		// Get the next stabilization point
		get_next_xy_coordinates(stabilization_x_, stabilization_y_);
	}
	closest_positions_.set_stabilization_coordinates(stabilization_x_, stabilization_y_);
}
void stabilization_smart_layer::process_pos(position* p_current_pos, position* p_previous_pos)
{
	//std::cout << "StabilizationSmartLayer::process_pos - Processing Position...";
	// if we're at a layer change, add the current saved plan
	if (p_current_pos->is_layer_change && p_current_pos->layer > 1)
	{
		//octolapse_log(octolapse_log::SNAPSHOT_PLAN, octolapse_log::VERBOSE, "Layer change detected.");
		is_layer_change_wait_ = true;
		// get distance from current point to the stabilization point
		
		standard_layer_trigger_distance_ = utilities::get_cartesian_distance(
			p_current_pos->x, p_current_pos->y,
			stabilization_x_, stabilization_y_
		);
	}
			
	if (is_layer_change_wait_ && !closest_positions_.is_empty())
	{
		bool can_add_saved_plan = true;
		if (stabilization_args_.height_increment != 0)
		{
			can_add_saved_plan = false;
			const double increment_double = p_current_pos->height / stabilization_args_.height_increment;
			unsigned const int increment = utilities::round_up_to_int(increment_double);
			if (increment > current_height_increment_)
			{
				// We only update the height increment if we've detected extrusion on a layer
				if (increment > 1.0 && !closest_positions_.is_empty())
				{
					current_height_increment_ = increment;
					can_add_saved_plan = true;
				}
				else
				{
					octolapse_log(octolapse_log::SNAPSHOT_PLAN, octolapse_log::WARNING, "Octolapse missed a layer while creating a snapshot plan due to a height restriction.");
				}
			}
		}
		if (can_add_saved_plan)
		{
			add_plan();

		}

	}
	//octolapse_log(octolapse_log::SNAPSHOT_PLAN, octolapse_log::VERBOSE, "Adding closest position.");
	closest_positions_.try_add(p_current_pos, p_previous_pos);
	last_tested_gcode_number_ = p_current_pos->gcode_number;
}

void stabilization_smart_layer::add_plan()
{
	trigger_position p_closest;
	if (closest_positions_.get_position(p_closest))
	{
		//std::cout << "Adding saved plan to plans...  F Speed" << p_saved_position_->f_ << " \r\n";
		snapshot_plan p_plan;
		double total_travel_distance;
		if (smart_layer_args_.smart_layer_trigger_type == trigger_type_snap_to_print)
		{
			total_travel_distance = 0;
		}
		else
		{
			total_travel_distance = p_closest.distance * 2;
		}

		p_plan.total_travel_distance = total_travel_distance;
		p_plan.saved_travel_distance = (standard_layer_trigger_distance_ * 2) - total_travel_distance;
		p_plan.distance_from_stabilization_point = p_closest.distance;
		p_plan.triggering_command_type = p_closest.type_position;
		p_plan.triggering_command_feature_type = p_closest.type_feature;
		// create the initial position
		p_plan.triggering_command = p_closest.pos.command;
		p_plan.start_command = p_closest.pos.command;
		p_plan.initial_position = p_closest.pos;
		p_plan.has_initial_position = true;
		const bool all_stabilizations_disabled = stabilization_args_.x_stabilization_disabled && stabilization_args_.y_stabilization_disabled;
		
		if (!(all_stabilizations_disabled || smart_layer_args_.smart_layer_trigger_type == trigger_type_snap_to_print))
		{
			double x_stabilization, y_stabilization;
			if (stabilization_args_.x_stabilization_disabled)
				x_stabilization = p_closest.pos.x;
			else
				x_stabilization = stabilization_x_;

			if (stabilization_args_.y_stabilization_disabled)
				y_stabilization = p_closest.pos.y;
			else
				y_stabilization = stabilization_y_;

			const snapshot_plan_step p_travel_step(&x_stabilization, &y_stabilization, NULL, NULL, NULL, travel_action);
			p_plan.steps.push_back(p_travel_step);
		}

		const snapshot_plan_step p_snapshot_step(NULL, NULL, NULL, NULL, NULL, snapshot_action);
		p_plan.steps.push_back(p_snapshot_step);

		// Only add a return position if we're not using snap to print
		if(smart_layer_args_.smart_layer_trigger_type != trigger_type_snap_to_print)
			p_plan.return_position = p_closest.pos;

		p_plan.file_line = p_closest.pos.file_line_number;
		p_plan.file_gcode_number = p_closest.pos.gcode_number;

		// Add the plan
		p_snapshot_plans_.push_back(p_plan);
		current_layer_ = p_closest.pos.layer;
		last_snapshot_initial_position_ = p_plan.initial_position;
		// only get the next coordinates if we've actually added a plan.
		update_stabilization_coordinates();
		
		// reset the saved positions
		reset_saved_positions();
		// Need to set the initial position after resetting the saved positions
		closest_positions_.set_previous_initial_position(last_snapshot_initial_position_);
		// Determine if we've missed a snapshot layer or height increment
		if(stabilization_args_.height_increment != 0)
		{
			if (current_height_increment_ > 2 && current_height_increment_ - 1 > last_snapshot_height_increment_)
				missed_snapshots_++;
		}
		else
		{
			if (current_layer_ - 1 > last_snapshot_layer_)
				missed_snapshots_++;
		}

		// update the last snapshot layer and increment
		last_snapshot_layer_ = current_layer_;
		last_snapshot_height_increment_ = current_height_increment_;
	}
	else
	{
		octolapse_log(octolapse_log::SNAPSHOT_PLAN, octolapse_log::WARNING, "No point snapshot position found for layer.");
		// reset the saved positions
		reset_saved_positions();
	}
	
	
	//std::cout << "Complete.\r\n";
}

void stabilization_smart_layer::reset_saved_positions()
{
	// Clear the saved closest positions
	closest_positions_.clear();
	// set the state for the next layer
	is_layer_change_wait_ = false;
	has_one_extrusion_speed_ = true;
	current_layer_saved_extrusion_speed_ = -1;
	standard_layer_trigger_distance_ = 0.0;
}

void stabilization_smart_layer::on_processing_complete()
{
	// If we were on 
	if (!closest_positions_.is_empty())
	{
		add_plan();
	}
	//std::cout << "Complete.\r\n";
}

std::vector<stabilization_quality_issue> stabilization_smart_layer::get_quality_issues()
{
	std::vector<stabilization_quality_issue> issues;

	// Detect quality issues and return as a human readable string.
	gcode_comment_processor* p_comment_processor = gcode_position_->get_gcode_comment_processor();
	if (this->smart_layer_args_.smart_layer_trigger_type == trigger_type_fast)
	{
		stabilization_quality_issue issue;
		issue.description = "You are using the 'Fast' smart trigger.  This could lead to quality issues.  If you are having print quality issues, consider using a 'high quality' or 'snap to print' smart trigger.";
		issue.issue_type = stabilization_quality_issue_fast_trigger;
		issues.push_back(issue);
	}
	else
	{
		if (this->smart_layer_args_.smart_layer_trigger_type == trigger_type_snap_to_print && !smart_layer_args_.snap_to_print_high_quality)
		{
			stabilization_quality_issue issue;
			issue.description = "In most cases using the 'High Quality' snap to print option will improve print quality, unless you are printing with vase mode enabled.";
			issue.issue_type = stabilization_quality_issue_snap_to_print_low_quality;
			issues.push_back(issue);
		}

		else if (p_comment_processor->get_comment_process_type() == comment_process_type_unknown)
		{
			stabilization_quality_issue issue;
			issue.description = "No print features were found in your gcode file.  This can reduce print quality significantly.  If you are using Slic3r or PrusaSlicer, please enable 'Verbose G-code' in 'Print Settings'->'Output Options'->'Output File'.";
			issue.issue_type = stabilization_quality_issue_no_print_features;
			issues.push_back(issue);
		}
	}

	return issues;
}
