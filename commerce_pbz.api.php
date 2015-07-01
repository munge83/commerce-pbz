<?php

/**
 * @file
 * Documents hooks provided by the Commerce PBZ module.
 */


/**
 * Let other modules load a different Commerce PBZ rule.
 *
 * @param $rule_name
 *   The machine name of the rule to be loaded.
 */
function hook_commerce_pbz_rule_alter(&$rule_name) {
  // The default rule provided by the module
  $rule_name = 'commerce_payment_commerce_pbz';
}
