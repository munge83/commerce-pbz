<?php

/**
 * Order details to reply to pbz payment.
 */
function commerce_pbz_checkout_details($payment_method = NULL, $debug_response = array()) {
  if (empty($debug_response)) {
    $response = $_REQUEST; 
  } else {
    $response = $debug_response;
  }
  
  $config = _commerce_pbz_get_rule_config();

  // If the order ID is invalid, log a message and exit.
  if (!is_numeric($response['order'])) {
    watchdog('commerce_pbz', 'Invalid Order ID @order. !response', array('@order' => $response['order'], '!response' => '<pre>'. print_r($response, TRUE) .'<pre>'), WATCHDOG_ERROR);
    return FALSE;
  }
  
  // If the store ID is invalid, log a message and exit.
  if ($response['store'] != $config['commerce_pbz_store']) {
    watchdog('commerce_pbz', 'Invalid Store ID @store. !response', array('@store' => $response['store'], '!response' => '<pre>'. print_r($response, TRUE) .'<pre>'), WATCHDOG_ERROR);
    return FALSE;
  }
  
  $order = commerce_order_load($response['order']);
  
  // If the order ID is not found, log a message and exit.
  if (!$order) {
    watchdog('commerce_pbz', 'Order ID @order not found. !response', array('@order' => $response['order'], '!response' => '<pre>'. print_r($response, TRUE) .'<pre>'), WATCHDOG_ERROR);
    return FALSE;
  }
  
  $wrapper = entity_metadata_wrapper('commerce_order', $order);  
  $amount = $wrapper->commerce_order_total->amount->value();
  $currency = $config['commerce_pbz_currency']; // @todo Shouldn't we take the currency from the store?
  
  // Only inform to the platform one item and total price.
  $items = 1;
  
  $shop = $config['commerce_pbz_store'];

  $ret = 'M' . $currency . $amount . "\n";
  $ret .= $items . "\n";
  $ret .= '1' . "\n";    // Reference number.
  $ret .= $shop . "\n";  // Description.
  $ret .= '1' . "\n";    // Quantity.
  $ret .= $amount . "\n"; // Price.
    
  // @todo only output when in debug mode
  watchdog('commerce_pbz', 'Sent order data to PBZ: !ret', array('!ret' => '<pre>' . check_plain(print_r($ret, TRUE)) . '</pre>'), WATCHDOG_DEBUG);
  print $ret;
}

/**
 * Payment method callback: checkout form submission.
 */
function commerce_pbz_redirect_page($debug_response = array()) {
  if (empty($debug_response)) {
    $response = $_REQUEST; 
  } else {
    $response = $debug_response;
  }

  $config = _commerce_pbz_get_rule_config();

  // Load the order
  $order = commerce_order_load(intval($response['order_number']));
  if (!$order) {
    return FALSE;
  }
  
  commerce_pbz_transaction($order, $response);  
  watchdog('commerce_pbz', 'Response processed for order number @order_number. !response', array('@order_number' => $order->order_number, '!response' => '<pre>' . check_plain(print_r($response, TRUE)) . '</pre>'), WATCHDOG_INFO);  
}

/**
 * Payment method callback: return from PBZ.
 */
function commerce_pbz_return($debug_response = array()) {
  if (empty($debug_response)) {
    $response = $_POST; 
  } else {
    $response = $debug_response;
  }
  
  // Load the order
  $order = commerce_order_load(intval($response['order_number']));
//echo '<div style="display: none;">';var_dump($response); echo "\n\r ################################################# \n\r"; var_dump(intval($response['order_number'])); echo "\n\r"; var_dump($order); echo '</div>';
  if (!$order) {
    return FALSE;
  }

  switch ($response['response_result']) {
    case 000:
      commerce_payment_redirect_pane_next_page($order);
      drupal_goto('checkout/' . $response['order_number'] . '/payment/return');      
      break;
    
    default:
      commerce_payment_redirect_pane_previous_page($order);
      drupal_goto('checkout/' . $response['order_number'] . '/payment/back');
      break;
  }
}